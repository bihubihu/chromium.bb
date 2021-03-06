# This file is auto-generated by the Perl DateTime Suite time zone
# code generator (0.07) This code generator comes with the
# DateTime::TimeZone module distribution in the tools/ directory

#
# Generated from /tmp/ympzZnp0Uq/northamerica.  Olson data version 2012c
#
# Do not edit this file directly.
#
package DateTime::TimeZone::America::Panama;
{
  $DateTime::TimeZone::America::Panama::VERSION = '1.46';
}

use strict;

use Class::Singleton 1.03;
use DateTime::TimeZone;
use DateTime::TimeZone::OlsonDB;

@DateTime::TimeZone::America::Panama::ISA = ( 'Class::Singleton', 'DateTime::TimeZone' );

my $spans =
[
    [
DateTime::TimeZone::NEG_INFINITY,
59611180688,
DateTime::TimeZone::NEG_INFINITY,
59611161600,
-19088,
0,
'LMT'
    ],
    [
59611180688,
60188764776,
59611161512,
60188745600,
-19176,
0,
'CMT'
    ],
    [
60188764776,
DateTime::TimeZone::INFINITY,
60188746776,
DateTime::TimeZone::INFINITY,
-18000,
0,
'EST'
    ],
];

sub olson_version { '2012c' }

sub has_dst_changes { 0 }

sub _max_year { 2022 }

sub _new_instance
{
    return shift->_init( @_, spans => $spans );
}



1;

