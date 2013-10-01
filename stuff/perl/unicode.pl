#!/usr/bin/env perl

use strict;
use warnings;
use utf8;

my $text = "@ARGV";
for ('a' .. 'z') {
   my $hex = chr (65216 + ord $_);
   $text =~ s/$_/$hex/g;
}
utf8::encode ($text);
print "$text\n";
