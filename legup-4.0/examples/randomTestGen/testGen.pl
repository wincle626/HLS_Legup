#!/usr/bin/perl
use strict;
use warnings;
use testSuite;


my $numArgs = $#ARGV + 1;

print "Number of arguements is $numArgs\n";
print "$ARGV[0]\n";

#my $TestSuite = testSuite->new(test_suite_name => "test_suite",template_file => "config.template");

#$TestSuite->readTemplateFile();
#$TestSuite->populateConfigFiles();