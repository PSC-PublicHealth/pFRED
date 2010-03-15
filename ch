#!/usr/bin/perl
use warnings;
use strict;
use Fcntl qw(:flock);

# Author: John Grefenstette
# Date: 15 Mar 2010
# Purpose: Update the params file

die "USAGE: ch parameter-name [ parameter-value [params-file] ]\n" if not @ARGV;
my ($param, $param_value, $params_file) = @ARGV;
$params_file = "params" if not $params_file;

# make sure given param is in the params.def file
my $x="$param = ";
my $line = `grep "^$x" params.def`;
die "$param not a valid parameter\n" if not $line;

# set locking semaphore

my $SEMAPHORE = ".params.lck";
open(S, ">$SEMAPHORE") || die "ch failed to obtain semaphore: ($!)\n";
flock(S, LOCK_EX);

# read current param file
open PARAMS_IN, $params_file or die "Can't read param file $params_file\n";
my @params_in = <PARAMS_IN>;
close PARAMS_IN;

# new params
my @params_out = ();

my $found = 0;
for my $pline (@params_in) {
  if ($pline =~ /^$param =/) {
    # replace the first instance of the param in the param list
    push @params_out, "$param = $param_value\n" if $param_value and not $found;
    $found = 1;
  }
  else {
    push @params_out, $pline;
  }
}
# add new param value to end of file if it was not found
push @params_out, "$param = $param_value\n" if $param_value and not $found;

# write out new params
open OUT, ">$params_file" or die "Can't write param file $params_file\n";
print OUT @params_out;
close OUT;

print "$param = $param_value\n" if $param_value;

# release semaphore
close S;

exit;
