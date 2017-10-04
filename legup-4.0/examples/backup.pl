#!/usr/bin/perl
#
# USAGE: backup.pl [backup-dir]
#
# This script finds all directories containing dg.exp files. Then
# backups up all of the following files:
my $fileTypes = "*.v transcript *.rpt *.c *.h config.tcl Makefile";
# The backup-dir argument is optional, if missing a directory will
# be created based on the current date and time

use warnings;
use strict;
use Cwd;

my $backupDir = $ARGV[0];# or die "Missing first argument: backup directory";

if (!$backupDir) {
    $backupDir = `date|sed -e "s/ /-/g"`;
    chomp $backupDir;
}


my $find_dg_exp = `find . -name dg.exp`;
my @dg_files = split /\n/, $find_dg_exp;

my $pwd = &Cwd::cwd();

foreach my $dg (@dg_files) {
    my $origDir = `dirname $dg`;
    chomp $origDir;
    print "Orig Dir: $origDir\n";

    my $newDir = "$backupDir/$origDir";
    print "New Dir: $newDir\n";

    system("mkdir -p $newDir");

    chdir $origDir or die("$!");
    my $ls_files = `ls $fileTypes`;
    # note: doesn't support spaces in file names
    my @files = split /\s+/, $ls_files;

    foreach my $file (@files) {
        system("cp $file $pwd/$newDir/");
    }
    chdir $pwd or die("$!");
}
