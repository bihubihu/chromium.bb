// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/quic/quic_unacked_packet_map.h"

#include "base/logging.h"
#include "base/stl_util.h"
#include "net/quic/quic_ack_notifier_manager.h"
#include "net/quic/quic_connection_stats.h"
#include "net/quic/quic_flags.h"
#include "net/quic/quic_utils_chromium.h"

using std::max;

namespace net {

QuicUnackedPacketMap::QuicUnackedPacketMap(
    AckNotifierManager* ack_notifier_manager)
    : largest_sent_packet_(0),
      largest_observed_(0),
      least_unacked_(1),
      bytes_in_flight_(0),
      pending_crypto_packet_count_(0),
      ack_notifier_manager_(ack_notifier_manager) {
}

QuicUnackedPacketMap::~QuicUnackedPacketMap() {
  QuicPacketNumber index = least_unacked_;
  for (UnackedPacketMap::iterator it = unacked_packets_.begin();
       it != unacked_packets_.end(); ++it, ++index) {
    delete it->retransmittable_frames;
    // Only delete all_transmissions once, for the newest packet.
    if (it->all_transmissions != nullptr &&
        index == *it->all_transmissions->rbegin()) {
      delete it->all_transmissions;
    }
  }
}

void QuicUnackedPacketMap::AddSentPacket(const SerializedPacket& packet,
                                         QuicPacketNumber old_packet_number,
                                         TransmissionType transmission_type,
                                         QuicTime sent_time,
                                         QuicByteCount bytes_sent,
                                         bool set_in_flight) {
  QuicPacketNumber packet_number = packet.packet_number;
  LOG_IF(DFATAL, largest_sent_packet_ >= packet_number) << packet_number;
  DCHECK_GE(packet_number, least_unacked_ + unacked_packets_.size());
  while (least_unacked_ + unacked_packets_.size() < packet_number) {
    unacked_packets_.push_back(TransmissionInfo());
    unacked_packets_.back().is_unackable = true;
  }

  TransmissionInfo info(packet.retransmittable_frames,
                        packet.packet_number_length, transmission_type,
                        sent_time, bytes_sent, packet.is_fec_packet);
  if (old_packet_number == 0) {
    if (packet.retransmittable_frames != nullptr &&
        packet.retransmittable_frames->HasCryptoHandshake() == IS_HANDSHAKE) {
      ++pending_crypto_packet_count_;
    }
  } else {
    TransferRetransmissionInfo(old_packet_number, packet_number,
                               transmission_type, &info);
  }

  largest_sent_packet_ = packet_number;
  if (set_in_flight) {
    bytes_in_flight_ += bytes_sent;
    info.in_flight = true;
  }
  unacked_packets_.push_back(info);
}

void QuicUnackedPacketMap::RemoveObsoletePackets() {
  while (!unacked_packets_.empty()) {
    if (!IsPacketUseless(least_unacked_, unacked_packets_.front())) {
      break;
    }
    PopLeastUnacked();
  }
}

void QuicUnackedPacketMap::TransferRetransmissionInfo(
    QuicPacketNumber old_packet_number,
    QuicPacketNumber new_packet_number,
    TransmissionType transmission_type,
    TransmissionInfo* info) {
  if (old_packet_number < least_unacked_ ||
      old_packet_number > largest_sent_packet_) {
    LOG(DFATAL) << "Old TransmissionInfo no longer exists for:"
                << old_packet_number << " least_unacked:" << least_unacked_
                << " largest_sent:" << largest_sent_packet_;
    return;
  }
  DCHECK_GE(new_packet_number, least_unacked_ + unacked_packets_.size());
  DCHECK_NE(NOT_RETRANSMISSION, transmission_type);

  TransmissionInfo* transmission_info =
      &unacked_packets_.at(old_packet_number - least_unacked_);
  RetransmittableFrames* frames = transmission_info->retransmittable_frames;
  transmission_info->retransmittable_frames = nullptr;
  LOG_IF(DFATAL, frames == nullptr)
      << "Attempt to retransmit packet with no "
      << "retransmittable frames: " << old_packet_number;

  // Only keep one transmission older than largest observed, because only the
  // most recent is expected to possibly be a spurious retransmission.
  while (transmission_info->all_transmissions != nullptr &&
         transmission_info->all_transmissions->size() > 1 &&
         *(++transmission_info->all_transmissions->begin()) <
             largest_observed_) {
    QuicPacketNumber old_transmission =
        *transmission_info->all_transmissions->begin();
    TransmissionInfo* old_info =
        &unacked_packets_[old_transmission - least_unacked_];
    // Don't remove old packets if they're still in flight.
    if (old_info->in_flight) {
      break;
    }
    old_info->all_transmissions->pop_front();
    // This will cause the packet be removed in RemoveObsoletePackets.
    old_info->all_transmissions = nullptr;
  }
  // Don't link old transmissions to new ones when version or
  // encryption changes.
  if (transmission_type == ALL_INITIAL_RETRANSMISSION ||
      transmission_type == ALL_UNACKED_RETRANSMISSION) {
    RemoveAckability(transmission_info);
  } else {
    if (transmission_info->all_transmissions == nullptr) {
      transmission_info->all_transmissions = new PacketNumberList();
      transmission_info->all_transmissions->push_back(old_packet_number);
    }
    transmission_info->all_transmissions->push_back(new_packet_number);
  }
  info->retransmittable_frames = frames;
  info->all_transmissions = transmission_info->all_transmissions;
  // Proactively remove obsolete packets so the least unacked can be raised.
  RemoveObsoletePackets();
}

void QuicUnackedPacketMap::ClearAllPreviousRetransmissions() {
  while (!unacked_packets_.empty() && least_unacked_ < largest_observed_) {
    // If this packet is in flight, or has retransmittable data, then there is
    // no point in clearing out any further packets, because they would not
    // affect the high water mark.
    TransmissionInfo* info = &unacked_packets_.front();
    if (info->in_flight || info->retransmittable_frames != nullptr) {
      break;
    }

    if (info->all_transmissions != nullptr) {
      if (info->all_transmissions->size() < 2) {
        LOG(DFATAL) << "all_transmissions must be nullptr or have multiple "
                    << "elements.  size:" << info->all_transmissions->size();
        delete info->all_transmissions;
        info->all_transmissions = nullptr;
      } else {
        LOG_IF(DFATAL, info->all_transmissions->front() != least_unacked_)
            << "The first element of all transmissions should be least unacked:"
            << least_unacked_ << " but is:" << info->all_transmissions->front();
        info->all_transmissions->pop_front();
        if (info->all_transmissions->size() == 1) {
          // Set the newer transmission's 'all_transmissions' entry to nullptr.
          QuicPacketNumber new_transmission = info->all_transmissions->front();
          TransmissionInfo* new_info =
              &unacked_packets_.at(new_transmission - least_unacked_);
          delete new_info->all_transmissions;
          new_info->all_transmissions = nullptr;
        }
      }
    }
    PopLeastUnacked();
  }
}

bool QuicUnackedPacketMap::HasRetransmittableFrames(
    QuicPacketNumber packet_number) const {
  DCHECK_GE(packet_number, least_unacked_);
  DCHECK_LT(packet_number, least_unacked_ + unacked_packets_.size());
  return unacked_packets_[packet_number - least_unacked_]
             .retransmittable_frames != nullptr;
}

void QuicUnackedPacketMap::NackPacket(QuicPacketNumber packet_number,
                                      QuicPacketCount min_nacks) {
  DCHECK_GE(packet_number, least_unacked_);
  DCHECK_LT(packet_number, least_unacked_ + unacked_packets_.size());
  unacked_packets_[packet_number - least_unacked_].nack_count = max(
      min_nacks, unacked_packets_[packet_number - least_unacked_].nack_count);
}

void QuicUnackedPacketMap::RemoveRetransmittability(
    QuicPacketNumber packet_number) {
  DCHECK_GE(packet_number, least_unacked_);
  DCHECK_LT(packet_number, least_unacked_ + unacked_packets_.size());
  TransmissionInfo* info = &unacked_packets_[packet_number - least_unacked_];
  PacketNumberList* all_transmissions = info->all_transmissions;
  if (all_transmissions == nullptr) {
    MaybeRemoveRetransmittableFrames(info);
    return;
  }
  // TODO(ianswett): Consider adding a check to ensure there are retransmittable
  // frames associated with this packet.
  for (QuicPacketNumber packet_number : *all_transmissions) {
    TransmissionInfo* transmission_info =
        &unacked_packets_[packet_number - least_unacked_];
    MaybeRemoveRetransmittableFrames(transmission_info);
    transmission_info->all_transmissions = nullptr;
  }
  delete all_transmissions;
}

void QuicUnackedPacketMap::RemoveAckability(TransmissionInfo* info) {
  DCHECK(info->retransmittable_frames == nullptr);
  info->is_unackable = true;
  PacketNumberList* all_transmissions = info->all_transmissions;
  if (all_transmissions == nullptr) {
    return;
  }
  for (QuicPacketNumber packet_number : *all_transmissions) {
    TransmissionInfo* transmission_info =
        &unacked_packets_[packet_number - least_unacked_];
    transmission_info->all_transmissions = nullptr;
    transmission_info->is_unackable = true;
  }
  delete all_transmissions;
}

void QuicUnackedPacketMap::MaybeRemoveRetransmittableFrames(
    TransmissionInfo* transmission_info) {
  if (transmission_info->retransmittable_frames == nullptr) {
    return;
  }
  if (transmission_info->retransmittable_frames->HasCryptoHandshake() ==
      IS_HANDSHAKE) {
    --pending_crypto_packet_count_;
  }
  delete transmission_info->retransmittable_frames;
  transmission_info->retransmittable_frames = nullptr;
}

void QuicUnackedPacketMap::IncreaseLargestObserved(
    QuicPacketNumber largest_observed) {
  DCHECK_LE(largest_observed_, largest_observed);
  largest_observed_ = largest_observed;
}

bool QuicUnackedPacketMap::IsPacketUsefulForMeasuringRtt(
    QuicPacketNumber packet_number,
    const TransmissionInfo& info) const {
  // Packet can be used for RTT measurement if it may yet be acked as the
  // largest observed packet by the receiver.
  return !info.is_unackable && packet_number > largest_observed_;
}

bool QuicUnackedPacketMap::IsPacketUsefulForCongestionControl(
    const TransmissionInfo& info) const {
  // Packet contributes to congestion control if it is considered inflight.
  return info.in_flight;
}

bool QuicUnackedPacketMap::IsPacketUsefulForRetransmittableData(
    const TransmissionInfo& info) const {
  // Packet may have retransmittable frames, or the data may have been
  // retransmitted with a new packet number.
  return info.retransmittable_frames != nullptr ||
         info.all_transmissions != nullptr;
}

bool QuicUnackedPacketMap::IsPacketUseless(QuicPacketNumber packet_number,
                                           const TransmissionInfo& info) const {
  return !IsPacketUsefulForMeasuringRtt(packet_number, info) &&
         !IsPacketUsefulForCongestionControl(info) &&
         !IsPacketUsefulForRetransmittableData(info);
}

bool QuicUnackedPacketMap::IsUnacked(QuicPacketNumber packet_number) const {
  if (packet_number < least_unacked_ ||
      packet_number >= least_unacked_ + unacked_packets_.size()) {
    return false;
  }
  return !IsPacketUseless(packet_number,
                          unacked_packets_[packet_number - least_unacked_]);
}

void QuicUnackedPacketMap::RemoveFromInFlight(QuicPacketNumber packet_number) {
  DCHECK_GE(packet_number, least_unacked_);
  DCHECK_LT(packet_number, least_unacked_ + unacked_packets_.size());
  TransmissionInfo* info = &unacked_packets_[packet_number - least_unacked_];
  if (info->in_flight) {
    LOG_IF(DFATAL, bytes_in_flight_ < info->bytes_sent);
    bytes_in_flight_ -= info->bytes_sent;
    info->in_flight = false;
  }
}

void QuicUnackedPacketMap::CancelRetransmissionsForStream(
    QuicStreamId stream_id) {
  QuicPacketNumber packet_number = least_unacked_;
  for (UnackedPacketMap::const_iterator it = unacked_packets_.begin();
       it != unacked_packets_.end(); ++it, ++packet_number) {
    RetransmittableFrames* retransmittable_frames = it->retransmittable_frames;
    if (retransmittable_frames == nullptr) {
      continue;
    }
    retransmittable_frames->RemoveFramesForStream(stream_id);
    if (retransmittable_frames->frames().empty()) {
      RemoveRetransmittability(packet_number);
    }
  }
}

bool QuicUnackedPacketMap::HasUnackedPackets() const {
  return !unacked_packets_.empty();
}

bool QuicUnackedPacketMap::HasInFlightPackets() const {
  return bytes_in_flight_ > 0;
}

const TransmissionInfo& QuicUnackedPacketMap::GetTransmissionInfo(
    QuicPacketNumber packet_number) const {
  return unacked_packets_[packet_number - least_unacked_];
}

QuicTime QuicUnackedPacketMap::GetLastPacketSentTime() const {
  UnackedPacketMap::const_reverse_iterator it = unacked_packets_.rbegin();
  while (it != unacked_packets_.rend()) {
    if (it->in_flight) {
      LOG_IF(DFATAL, it->sent_time == QuicTime::Zero())
          << "Sent time can never be zero for a packet in flight.";
      return it->sent_time;
    }
    ++it;
  }
  LOG(DFATAL) << "GetLastPacketSentTime requires in flight packets.";
  return QuicTime::Zero();
}

size_t QuicUnackedPacketMap::GetNumUnackedPacketsDebugOnly() const {
  size_t unacked_packet_count = 0;
  QuicPacketNumber packet_number = least_unacked_;
  for (UnackedPacketMap::const_iterator it = unacked_packets_.begin();
       it != unacked_packets_.end(); ++it, ++packet_number) {
    if (!IsPacketUseless(packet_number, *it)) {
      ++unacked_packet_count;
    }
  }
  return unacked_packet_count;
}

bool QuicUnackedPacketMap::HasMultipleInFlightPackets() const {
  size_t num_in_flight = 0;
  for (UnackedPacketMap::const_reverse_iterator it = unacked_packets_.rbegin();
       it != unacked_packets_.rend(); ++it) {
    if (it->in_flight) {
      ++num_in_flight;
    }
    if (num_in_flight > 1) {
      return true;
    }
  }
  return false;
}

bool QuicUnackedPacketMap::HasPendingCryptoPackets() const {
  return pending_crypto_packet_count_ > 0;
}

bool QuicUnackedPacketMap::HasUnackedRetransmittableFrames() const {
  for (UnackedPacketMap::const_reverse_iterator it =
           unacked_packets_.rbegin(); it != unacked_packets_.rend(); ++it) {
    if (it->in_flight && it->retransmittable_frames) {
      return true;
    }
  }
  return false;
}

QuicPacketNumber QuicUnackedPacketMap::GetLeastUnacked() const {
  return least_unacked_;
}

void QuicUnackedPacketMap::PopLeastUnacked() {
  ack_notifier_manager_->OnPacketRemoved(least_unacked_);

  unacked_packets_.pop_front();
  ++least_unacked_;
}

}  // namespace net
