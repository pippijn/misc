#!/usr/bin/perl

use warnings;
use strict;

sub cleanup {
   1 while 0
   | $_[0] =~ s/\bord chr //g
   | $_[0] =~ s/\blc uc //g
   | $_[0] =~ s/\buc lc //g
   | $_[0] =~ s/\bchr ord (?=(?:[lu]c )*chr )//g
   | $_[0] =~ s/\b(ord (?:[lu]c )*)chr ord /$1/g
   | $_[0] =~ s/\bchr ord (?=ord q (\S).?\1)//g
   ;
}

sub there {
   $_[0] and @{ $_[0] }
}

sub mkoct {
   my ($gen, $k) = @_;
   my $alt = chr sprintf '%o', ord $k;
   return if there $gen->{$k} or !there $gen->{$alt};
   push @{ $gen->{$k} }, map "oct $_", @{ $gen->{$alt} };
}

my %h;
while (<DATA>) {
   chomp;
   push @{ $h{substr $_, 0, 1} }, $_;
}

my %gen = (
   '0' => ['ord ord q qq'],
   ' ' => ['chr ord q q q', 'chr ord q m m'],
);

for my $k (keys %h) {
   for my $v (@{ $h{$k} }) {
      if ($v !~ /(.).*\1\z/) {
         my $end = substr $v, -1, 1;
         for my $ps (@{ $h{$end} }) {
            if ($ps !~ /^(.).*\1/) {
               if (length($ps) > 1) {
                  push @{ $gen{substr $ps, 1, 1} }, "ord q $ps $v";
               } else {
                  for my $alt (map @$_, values %h) {
                     push @{ $gen{' '} }, "ord q $ps $alt $v"
                        if index ($alt, $end) == -1
                  }
               }
            }
         }
      }
   }
}

for my $k (grep /[a-z]/, keys %gen) {
   push @{ $gen{uc $k} }, map "ord uc chr $_", @{ $gen{$k} };
}

for my $k (keys %gen) {
   my $n = substr ord($k), 0, 1;
   push @{ $gen{$n} }, map "ord $_", @{ $gen{$k} };
}

for my $n (grep /[0-9]/, keys %gen) {
   push @{ $gen{substr ord ($n), 0, 1 }}, map "ord $_", @{ $gen{$n} };
}

for my $k (keys %gen) {
   my $n = ord $k;
   if ($n =~ /^[0-7]+\z/) {
      my $alt = oct $n;
      push @{ $gen{chr $alt} }, map "oct $_", @{ $gen{$k} };
   }
   if ($n =~ /^[[:xdigit:]]+\z/) {
      my $alt = hex $n;
      next if $alt > 126;
      push @{ $gen{chr $alt} }, map "hex $_", @{ $gen{$k} };
   }
}

for my $c (qw(F d j)) {
   my $o = $c ^ ' ';
   if (!there $gen{$c} and @{ $gen{$o} }) {
      my $u = uc($c) eq $c;
      push @{ $gen{$c} }, map "ord " . ($u ? 'uc' : 'lc') .  " chr $_", @{ $gen{$o} };
   }
}

push @{ $gen{Z} }, map "oct hex $_",    @{ $gen{T} } if !there $gen{Z} and there $gen{T};
push @{ $gen{z} }, map "ord lc chr $_", @{ $gen{Z} } if !there $gen{z} and there $gen{Z};

for my $x (qw{! ( ' # $ % @}, (map chr, qw[30 24 20 16 14 12 10 8])) {
   mkoct \%gen, $x;
}

for my $v (values %gen) {
   for my $x (@$v) {
      $x = "chr $x";
      cleanup $x;
   }
}

my @fin;
for my $k (keys %gen) {
   $fin[ord $k] = (sort {length $a <=> length $b} @{ $gen{$k} })[0];
}
my %words;
for my $x (@fin) {
   $x = [defined $x ? split ' ', $x : ()];
   @words{@$x} = ();
}

{
   my $i = 0;
   for my $k (sort keys %words) {
      $words{$k} = $i++;
   }
}

for my $x (@fin) {
	$x = join '', map chr $words{$_}, @$x;
}

print "#!/usr/bin/perl\nuse warnings;\nuse strict;\n\n",
      "my \@words = qw(\n",
      (join ' ', sort keys %words),
      "\n);\n\nmy \@gen = (\n",
      (map qq{\t"} . join('', map sprintf('\\x%02x', ord), split //) . qq{",\n}, @fin),
      ");\n",
<<'EOF',
ARG:
for my $t (@ARGV) {
   my @s;
   for my $c (map ord, split //, $t) {
      unless ($gen[$c]) {
         print STDERR "error: don't know how to encode [${\chr $c}]\n";
         next ARG;
      }
      push @s, 'print ' . join ' ', map $words[ord], split //, $gen[$c];
   }
   print join(" and ", @s), "\n";
}
EOF
;

__DATA__
abs
accept
alarm
and
bind
binmode
bless
caller
chdir
chmod
chomp
chop
chown
chr
chroot
close
closedir
cmp
connect
continue
cos
crypt
dbmclose
dbmopen
defined
delete
die
do
dump
each
else
elsif
endgrent
endhostent
endnetent
endprotoent
endpwent
endservent
eof
eq
eval
exec
exists
exit
exp
fcntl
fileno
flock
for
foreach
fork
format
formline
ge
getc
getgrent
getgrgid
getgrnam
gethostbyaddr
gethostbyname
gethostent
getlogin
getnetbyaddr
getnetbyname
getnetent
getpeername
getpgrp
getppid
getpriority
getprotobyname
getprotobynumber
getprotoent
getpwent
getpwnam
getpwuid
getservbyname
getservbyport
getservent
getsockname
getsockopt
glob
gmtime
goto
grep
gt
hex
if
index
int
ioctl
join
keys
kill
last
lc
lcfirst
le
length
link
listen
local
localtime
lock
log
lstat
lt
m
map
mkdir
msgctl
msgget
msgrcv
msgsnd
my
ne
next
no
not
oct
open
opendir
or
ord
our
pack
package
pipe
pop
pos
print
printf
prototype
push
q
qq
qr
quotemeta
qw
qx
rand
read
readdir
readline
readlink
readpipe
recv
redo
ref
rename
require
reset
return
reverse
rewinddir
rindex
rmdir
s
scalar
seek
seekdir
select
semctl
semget
semop
send
setgrent
sethostent
setnetent
setpgrp
setpriority
setprotoent
setpwent
setservent
setsockopt
shift
shmctl
shmget
shmread
shmwrite
shutdown
sin
sleep
socket
socketpair
sort
splice
split
sprintf
sqrt
srand
stat
study
sub
substr
symlink
syscall
sysopen
sysread
sysseek
system
syswrite
tell
telldir
tie
tied
time
times
tr
truncate
uc
ucfirst
umask
undef
unless
unlink
unpack
unshift
untie
until
use
utime
values
vec
wait
waitpid
wantarray
warn
while
write
x
xor
y
