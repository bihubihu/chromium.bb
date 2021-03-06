// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/login/test/app_window_waiter.h"

#include "extensions/browser/app_window/app_window.h"

namespace chromeos {

AppWindowWaiter::AppWindowWaiter(extensions::AppWindowRegistry* registry,
                                 const std::string& app_id)
    : registry_(registry), app_id_(app_id) {
  registry_->AddObserver(this);
}

AppWindowWaiter::~AppWindowWaiter() {
  registry_->RemoveObserver(this);
}

extensions::AppWindow* AppWindowWaiter::Wait() {
  window_ = registry_->GetCurrentAppWindowForApp(app_id_);
  if (window_)
    return window_;

  wait_type_ = WAIT_FOR_ADDED;
  run_loop_.reset(new base::RunLoop);
  run_loop_->Run();

  return window_;
}

extensions::AppWindow* AppWindowWaiter::WaitForShown() {
  window_ = registry_->GetCurrentAppWindowForApp(app_id_);
  if (window_ && !window_->is_hidden())
    return window_;

  wait_type_ = WAIT_FOR_SHOWN;
  run_loop_.reset(new base::RunLoop);
  run_loop_->Run();

  return window_;
}

void AppWindowWaiter::OnAppWindowAdded(extensions::AppWindow* app_window) {
  if (wait_type_ != WAIT_FOR_ADDED || !run_loop_ || !run_loop_->running())
    return;

  if (app_window->extension_id() == app_id_) {
    window_ = app_window;
    run_loop_->Quit();
  }
}

void AppWindowWaiter::OnAppWindowShown(extensions::AppWindow* app_window,
                                       bool was_hidden) {
  if (wait_type_ != WAIT_FOR_SHOWN || !run_loop_ || !run_loop_->running())
    return;

  if (app_window->extension_id() == app_id_) {
    window_ = app_window;
    run_loop_->Quit();
  }
}

}  // namespace chromeos
