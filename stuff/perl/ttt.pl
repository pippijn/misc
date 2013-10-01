#!/usr/bin/env perl

use strict;
use warnings;
use utf8;

use Net::IRC;

my $irc = new Net::IRC;

my $field = [
   [ 0, 0, 0 ],
   [ 0, 0, 0 ],
   [ 0, 0, 0 ],
];

my $conn = $irc->newconn (
   Server   => 'irc.one09.net',
   Port     => 6667,
   Nick     => 'ttt',
   Username => 'ttt',           # ident
   Ircname  => 'Tic-Tac-Toe',   # gecos
) or die "irctest: Can't connect to IRC server.\n";

$conn->add_handler ('public', \&on_public);
$conn->add_global_handler (376, \&on_connect);

$irc->start;

sub won {
   return 0;
}

sub drawfield {
   my $won = won;
   return $won if $won;

   my $retval = "   1   2   3\n--------------\n";
   for my $x (1 .. 3) {
      $retval .= "$x|";
      for my $y (1 .. 3) {
         if ($field->[$x - 1]->[$y - 1] == 0) {
            $retval .= "   ";
         } elsif ($field->[$x - 1]->[$y - 1] == 1) {
            $retval .= " x ";
         } elsif ($field->[$x - 1]->[$y - 1] == 2) {
            $retval .= " o ";
         }
         $retval .= "|"
            if $y < 3;
      }
      $retval .= "\n--------------"
         if $x < 3;
      $retval .= "\n";
   }

   $retval;
}

sub reset_board {
   $field = [
      [ 0, 0, 0 ],
      [ 0, 0, 0 ],
      [ 0, 0, 0 ],
   ];
}

sub on_connect {
   my ($self) = @_;
   $self->join("#ttt");
}

sub on_public {
   my ($self, $event) = @_;
   my $nick = $event->nick;
   my @to = $event->to;
   my ($arg) = $event->args;
   
   return unless $arg =~ /^!/;

   return reset_board && $self->privmsg ([@to], "Spielbrett genullt")
      if ($arg eq "!reset");

   if ($arg eq "!feld") {
      for (split /\n/, drawfield) {
         $self->privmsg ([@to], $_);
      }

      return;
   }

   my ($y, $x) = split /:/, $arg;
   $y =~ s/!//;
   $x += 0; $y += 0;

   if ($x < 1 or $x > 3 or $y < 1 or $y > 3) {
      $self->privmsg ([@to], "Ups.. feld $y:$x gibt es nicht");
      return;
   }

   if ($nick eq "pippijn") {
      if ($field->[$x - 1]->[$y - 1]) {
         $self->privmsg ([@to], "Da ist schon ein zeichen");
         return;
      } else {
         $field->[$x - 1]->[$y - 1] = 2;
      }
   } elsif ($nick eq "carmen") {
      if ($field->[$x - 1]->[$y - 1]) {
         $self->privmsg ([@to], "Da ist schon ein zeichen");
         return;
      } else {
         $field->[$x - 1]->[$y - 1] = 1;
      }
   }

   for (split /\n/, drawfield) {
      $self->privmsg ([@to], $_);
   }
}
