#!/usr/bin/env perl

use strict;
use warnings;
use utf8;

my @code = do { open my $fh, "<merged.cpp" or die "merged.cpp: $!"; <$fh> };
my $file;

for my $line (@code) {
   my ($outfile) = $line =~ /^\/\/>>>>>>>>>>> (.+) <<<<<<<<<<<\/\/$/;
   if ($outfile) {
      $file = $outfile;
      close CODE unless $file;
      open CODE, ">$file";
      next;
   }
   print CODE $line;
}
