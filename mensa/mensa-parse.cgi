#!/usr/bin/perl

use strict;
use warnings FATAL => 'all';
use utf8;

use Time::HiRes 'time';
use CGI;
use CGI::Carp 'fatalsToBrowser';
use Storable;

my $begin = time;

my @output;

my %food;
my $food;

my $cachefile = "/tmp/mensa-plan.pst";

my $cache;
if (-f $cachefile) {
   $cache = retrieve $cachefile;
   %food = %{ $cache->{HASH} };
   $food = $cache->{ARRAY};

   use File::stat;

   my $sb = stat $cachefile;
   if (time > $sb->mtime + 86400) {
      unlink $cachefile;
   }
}

my $q = new CGI;
$q->charset ("UTF-8");

my $raw;
my (my $m, $a) = $q->param ("keywords");
if ($a) {
   $raw = $a eq "raw";
}

$m = "list"
   unless $m;

my $key = $raw ? "raw" : "cur";
if (exists $cache->{$m}->{$key}) {
   @output = @{ $cache->{$m}->{$key} }
} else {
   require "Mensa.pm";
   @output = Mensa->build ($cache, $m, $raw);

   $cache->{$m}->{$key} = [@output];

   store $cache, $cachefile;
}

print @output;

printf "\n\n<!-- Generated in %f seconds -->\n", time - $begin
   if $m eq "list";
