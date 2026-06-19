#!/usr/bin/perl
use strict;
use warnings;

my $module = "KERNEL_CRASH_TESTER";
print "<5>[  $module  ] Injecting illegal instruction sequences into running memory spaces...\n";

# Simulate writing directly to the syscall vector triggers with garbage code parameters
my $syscall_trigger_fd = "/proc/sys/kernel/debug_crash";

if (-e $syscall_trigger_fd) {
    open(my $fh, '>', $syscall_trigger_fd) or die "Cannot open hook interface line: $!";
    # Write a token to force a null pointer dereference crash test in kernel space
    print $fh "NULL_DEREF";
    close($fh);
} else {
    print "<4>[  $module  ] Warning: Virtual crash interface nodes are missing inside devFS tree frameworks.\n";
}