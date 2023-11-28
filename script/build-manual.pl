#! /usr/bin/env perl
use strict;
use warnings;
use Pod::Man;
use Getopt::Long;

GetOptions(
	'as=s'      => \(my $opt_as),
	'version=s' => \(my $opt_version),
	'help|h'    => sub { print "Usage:  $0 [--as=pod|man] header.pod *.c footer.pod\n\n"; exit 1; },
) or die;

# Merge all identically named head1 and head2 sections.  This allows multiple source files
# to contribute to the same section of documentation without needing to rearrange the code
# to match, and lets the manual.head.pod determine where everything will go in the document.
my @head1;
my %head1_by_name;
my $current_h1;
my $current_h2;
my $current_pod;
while (<>) { # there are nicer ways to do this, but I'm aiming for no non-core deps
	if ($_ =~ m{^(/\*)?=(\w+)\s*(.*?)\s*$}) {
		$_= substr($_, 2) if $1;
		if ($2 eq 'head1') {
			$current_h2= undef;
			$current_h1= $head1_by_name{$3} ||= do {
				my $h1= { name => $3, pod => '', head2 => [], head2_by_name => {} };
				push @head1, $h1;
				$h1;
			};
			$current_pod= \$current_h1->{pod};
		}
		elsif ($2 eq 'head2') {
			defined $current_h1 or die "got head2 before head1";
			$current_h2= $current_h1->{head2_by_name}{$3} ||= do {
				my $h2= { name => $3, pod => '', items => [] };
				push @{ $current_h1->{head2} }, $h2;
				$h2;
			};
			$current_pod= \$current_h2->{pod};
		}
		elsif ($2 eq 'item' and $current_h1->{name} eq 'COMMANDS' && defined $current_h2) { # only capture =item if it was part of COMMAND section
			my $i= { name => $3, pod => '' };
			push @{ $current_h2->{items} }, $i;
			$current_pod= \$i->{pod};
		}
		# if "cut", end the current part
		elsif ($2 eq 'cut') {
			$current_pod= undef;
		}
		elsif (!defined $current_pod) {
			die "Found POD directive with no current head1/head2/item: $_";
		}
		else {
			$$current_pod .= $_;
		}
	}
	elsif ($current_pod) {
		$$current_pod .= $_;
	}
}

# Now re-flatten the sections into a document
my $doc= '';
for my $h1 (@head1) {
	$doc .= "=head1 $h1->{name}\n\n$h1->{pod}\n\n";
	for my $h2 (@{ $h1->{head2} }) {
		$doc .= "=head2 $h2->{name}\n\n$h2->{pod}\n\n";
		if (@{ $h2->{items} }) {
			$doc .= "=over\n\n";
			for my $item (@{ $h2->{items} }) {
				$doc .= "=item $item->{name}\n\n$item->{pod}\n\n";
			}
			$doc .= "=back\n\n";
		}
	}
}

# remove redundant empty lines
$doc =~ s/\n\n\n+/\n\n/g;

# Either output POD or run it through Pod::Man
if (lc($opt_as) eq 'pod') {
	print $doc;
}
elsif (lc($opt_as) eq 'man') {
	open( my $doc_fh, '<', \$doc ) or die;
	Pod::Man->new(
		name => 'CmdlineGL',
		release => $opt_version,
		section => 1,
		center => 'General Commands',
		errors => 'die',
	)->parse_from_file($doc_fh);
}
elsif (lc($opt_as) eq 'html') {
	require Pod::Html;
	require File::Temp;
	my $tmp= File::Temp->new;
	$tmp->print($doc);
	$tmp->seek(0,0);
	my $html_tmp= File::Temp->new;
	Pod::Html::pod2html("--infile=$tmp", "--outfile=$html_tmp", "--title=CmdlineGL");
	$html_tmp->seek(0,0);
	local $/= undef;
	my $html= <$html_tmp>;
	my $html_head= <<END;
<!DOCTYPE html>
<html>
<head>
  <title>CmdlineGL</title>
  <link rel="stylesheet" href="yamb.css" type="text/css" />
  <meta http-equiv="content-type" content="text/html; charset=utf-8" />
  <style>
  html { margin: 0 2%; }
  body { margin: .5em auto 2em auto; max-width: 60em; }
  dl { margin: 0 .5em 0 0; }
  dt { margin: 0; font-family: "Consolas", "Inconsolata", "Courier New", monospace; }
  dd { margin: .5em 1em 1em 2em; }
  ul, ol { padding-left: 2.5em; list-style-position: outside; }
  #index { float: right; margin: 2em 1em;}
  #index, #index ul { list-style-type: none; padding-left: 1em; }
  code { max-width: 50em; }
  pre { margin: .5em 0; }
  </style>
</head>
END
	$html =~ s,.*?</head>,$html_head,s;
	print $html;
}
else {
	die "Unknown format $opt_as"
}
