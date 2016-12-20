#!perl
# sample script for scriptgui interface

# import packages and define globals
use strict;
# check if we were called directly or imported from another script
unless($#ARGV > 0) {
    print("usage: $0 input*csv col=1;2;3;4 logical_button@1 logical_false@0 table#tsheet*csv#tcol:tsheet#tchoice=1;2;3;4 output*csv out_log*log");
    exit;
}

print "$0\n\n$^V\n\n@ARGV\n\n";

1;
