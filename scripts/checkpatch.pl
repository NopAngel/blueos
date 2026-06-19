#!/usr/bin/perl
use strict;
use warnings;

my $module = "CHECKPATCH";
print "<6>[  $module  ] Scanning source file alignment parameters...\n";

my $errors = 0;
my $warnings = 0;

while (my $line = <>) {
    my $line_num = $.;
    
    # Rule 1: Check for hardcoded 8-space tabs vs spaces alignment
    if ($line =~ /^[ ]{8}/) {
        print "<4>[  $module  ] WARNING: Line $line_num uses 8 spaces instead of a native Tab character.\n";
        $warnings++;
    }
    
    # Rule 2: Trailing whitespace check guard
    if ($line =~ /\s+$/ && $line !~ /^\s*$/) {
        print "<3>[  $module  ] ERROR: Trailing whitespace detected at end of line $line_num.\n";
        $errors++;
    }
}

print "<6>[  $module  ] Scan complete. Found $errors errors, $warnings warnings.\n";
exit($errors > 0 ? 1 : 0);