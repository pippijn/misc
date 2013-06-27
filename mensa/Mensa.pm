package Mensa;

use strict;
use warnings FATAL => 'all';
use utf8;

use CGI;
use Date::Language;
use Date::Parse;
use Encode;
use HTML::TreeBuilder;
use LWP::Simple;
use Storable;


sub clamp_day {
   my ($sec, $min, $hour) = localtime $_[0];
   $_[0] - $sec
         - $min * 60
         - $hour * 3600
}

sub current {
   time > (clamp_day time) + 3600 * 14                # after 14:00Z
      ? (clamp_day $_[0]) == (clamp_day time) + 86400 # show tomorrow's meals
      : (clamp_day $_[0]) == (clamp_day time)         # else show today's meals
}

sub fmttime {
   use POSIX;
   strftime "%a %d.%m.", localtime $_[0]
}

my $raw;
sub today {
   my ($food) = @_;

   return $food
      if $raw;

   exists $food->{meal}
      ? { meal => [map { $_->{date} = fmttime $_->{date}; $_ } grep { current $_->{date} } @{ $food->{meal} }] }
      : { map { (fmttime $_) => $food->{$_} } grep { current $_ } keys %$food }
}

my @output;
sub outln {
   push @output, @_
}

my $q = new CGI;
sub header {
   outln $q->header ($_[0] . "; charset=UTF-8")
}


sub build {
   (undef, my ($cache, $m), $raw) = @_;

   my %food;
   my $food;

   $q->charset ("UTF-8");

   my $cachefile = "/tmp/mensa-plan.pst";
   my $src = "http://www.studentenwerk-karlsruhe.de/en/essen/?view=ok&STYLE=popup_plain&c=moltke&p=1";

   if (exists $cache->{HASH}) {
      %food = %{ $cache->{HASH} };
      $food = $cache->{ARRAY};
   } else {
      my $data = HTML::TreeBuilder->new_from_content (
         do { my $d = get $src; decode "utf8", $d }
      );

      my ($mensaname, @hs) = $data->find_by_attribute ("id", "platocontent")->find_by_tag_name ("h1");

      for my $h (@hs) {
         my $date = $h->content->[0];
         my $table = $h->right;

         {
            my $year = (localtime time)[5] + 1900;
            $date =~ s/(?:Mo|Tu|We|Th|Fr) (\d+)\.(\d+)\./$year-$2-$1/;
            $date = Date::Language->new ('German')->str2time ($date);
         }

         for my $tr (grep { $_->tag eq "tr" } @{ $table->content }) {
            my ($loc, $table) = @{ $tr->content };
            $loc = $loc->content->[0]->content->[0];
         
            utf8::encode $loc;
            
            for my $tr (@{ $table->content->[0]->content }) {
               my ($name, $price) = @{ $tr->content };
               $name = $name->as_trimmed_text;
               $price = $price->as_trimmed_text;

               utf8::encode $name;
               utf8::encode $price;

               $food{$date}{$loc}{$name} = $price;
               push @{ $food->{meal} }, {
                  date => $date,
                  loc => $loc,
                  name => $name,
                  price => $price,
               }
            }
         }
      }

      $data->delete;

      $cache->{HASH}  = \%food;
      $cache->{ARRAY} = $food;
   }

   my %dispatch;
   %dispatch = (
      json => sub {
         use JSON;
         header 'application/json';
         outln (JSON->new->pretty->canonical->encode (today \%food));
      },
      xml => sub {
         use XML::Simple;
         header 'application/xml';
         outln (XMLout ((today $food),
            NoAttr => 1,
            RootName => "meals",
         ))
      },
      rss => sub {
         use XML::RSS;
         undef $raw;
         header 'application/rss+xml';
         my $rss = new XML::RSS;
         my %today = %{ today \%food };
         while (my ($date, $meals) = each %today) {
            $rss->channel (
               title => "Mensaplan",
               description => "Mensaplan fuer die Mensa Moltkestrasse ($date)",
            );
            for my $meal (sort keys %$meals) {
               my $data = $meals->{$meal};
               my $desc = join "<br/>", map { "$_: $data->{$_}" } keys %$data;
               utf8::decode $meal;
               utf8::decode $desc;
               my %arg = (
                  title => $meal,
                  description => $desc,
                  link => $src,
               );
               $rss->add_item (%arg);
            }
         }
         outln $rss->as_string
      },
      perl => sub {
         use Data::Dumper;
         local $Data::Dumper::Indent = 1;
         local $Data::Dumper::Terse = 1;
         header 'text/plain';
         outln (Dumper today \%food)
      },
      yaml => sub {
         use YAML;
         header 'text/plain';
         outln (Dump today \%food)
      },
      list => sub {
         outln $q->header,
               $q->start_html ("Mensaplan Formate"),
               $q->h1 ("Mensaplan Formate"),
               $q->span ("Die <a href='$src'>Quelle</a> der folgenden Ausgaben wird ein Mal am Tag aktualisiert."),
               $q->ul (
                  map {
                     "<li><a href='?$_'>$_</a>" . (
                        $_ ne "rss" ? " (<a href='?$_+raw'>raw</a>)" : ""
                     ) . "</li>"
                  } sort grep { $_ ne "list" } keys %dispatch
               ),
               $q->end_html
      },
   );

   $m = "list"
      unless $m and exists $dispatch{$m};

   $dispatch{$m}->();

   @output
}
