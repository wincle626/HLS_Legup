#!/usr/bin/perl
#
# USAGE: benchmark.pl --family <family> --key <key> --prefix <prefix>
#

use warnings;
#use strict;
use Cwd;
use Data::Dumper;
use Getopt::Long;

# --family <family>
#my $family = 'CycloneII';
my $family = 'StratixIV';

# --key <key>
# This is passed by buildbot to indicate the type of graph we are collecting
# results for.
# ie. ./benchmark.pl LEs
# Will print out lines:
#   *RESULT all: dhrystone= 5165
#   *RESULT dhrystone: LEs= 5165
# The * indicates the result is 'important' and should be displayed on the buildbot waterfall
# The line must be formatted as:
#     <*>RESULT <graph_name>: <trace_name>= <value> <units>
my $performance_key = '';

# This is used to append a prefix to all result names:
# ie. ./benchmark.pl LEs tiger_
#   *RESULT tiger_all: tiger_dhrystone= 5165
#   *RESULT tiger_dhrystone: LEs= 5165
my $prefix = '';

GetOptions ("family=s" => \$family,
            "key=s" => \$performance_key,
            "prefix=s" => \$prefix
           ) || die;

my $stratix = 0;
$stratix = 1 if ($family =~ /stratix/i);

#my @benchmarks = qw|divstore mandelbrot primestore df hash1k los2|;
my @benchmarks = qw|divstore mandelbrot primestore hash1k los2 df|;
#my @benchmarks = qw|divstore mandelbrot primestore|;
my @types = qw|single pthreads pthreads_pipeline hybrid_2 hybrid_3 hybrid_4|;
#my @types = qw|single|;

my $pwd = &Cwd::cwd();

#my @metric_keys = qw(time cycles Fmax LEs areaDelay regs comb mults membits logicUtil);
my @metric_keys = qw(time cycles Fmax LEs regs comb mults membits logicUtil);


my %metric_units = (
    'time' => 'us',
    'Fmax' => 'MHz',
);

my %allmetrics;
my %quartusWarnings;
#%results;


#initialize all values to 0
foreach my $name (@benchmarks) {
    foreach my $type (@types) {
        $results{$name, $type} = "N/A";
        foreach my $key (@metric_keys) {
            $allmetrics{$name, $type}->{$key} = 0;
        }
    }
}

foreach my $name (@benchmarks) {
    chdir $name or die "$!";

    foreach my $type (@types) {
        if (-d $type) {
            chdir $type or die "$!";
            # $quartusWarnings{$name} = checkForWarnings($name);
            $allmetrics{$name, $type} = parse($name, $type);
            chdir ".." or die "Failed to go to parent directory: $!";
        }
    }

    chdir $pwd or die "$!";
}

foreach my $name (@benchmarks) {
    foreach my $type (@types) {
        print "$results{$name, $type}\n";
    }
}
#print
#foreach my $name (@benchmarks) {
#    foreach my $type (@types) {
#    	foreach my $key (@metric_keys) {
#	    my $value = $allmetrics{$name, $type}->{$key};
#	    print "$name $type ";
#	    print "$key = $value\n";
#	}
#    }
#}

# calculate geomean
my %geomean;
foreach my $type (@types) {
    foreach my $key (@metric_keys) {
        $geomean{$type, $key} = 1;
    }
}

my $count = 0;

foreach my $type (@types) {
    foreach my $name (@benchmarks) {
        #if ($allmetrics{$name, $type}->{Fmax} ne "N/A") {
        if ($allmetrics{$name, $type}->{Fmax} != 0) {
            $count++;
            foreach my $key (@metric_keys) {
                my $value = $allmetrics{$name, $type}->{$key};
                # for 0 entries just assume 1. otherwise geomean will just equal 0.
                if ($value != 0) {
                    $geomean{$type, $key} *= $value;
                }
            }
        }
    }
    $count = 1 if ($count == 0); # avoid division by zero
    foreach my $key (@metric_keys) {
        $geomean{$type, $key} **= 1.0/$count;
    }
}

#my @metric_keys = qw(time cycles Fmax LEs regs comb mults membits logicUtil);
#calcuate ratios between types per metric
my $ratios;
foreach my $name (@benchmarks) {
    foreach my $key (@metric_keys) {
        foreach my $type (@types) {
            if ($type eq "single" ) {
                $ratios{$name, $key, $type} = 1;
            } else {
                #to get speed up
                if (($key eq "time") || ($key eq "cycles")) {
                    #if that type of benchmark doesn't exist
                    if ($allmetrics{$name, $type}->{$key} == 0) {
                        my $value = 0;
                        $ratios{$name, $key, $type} = sprintf("%.2f", $value);
                    } else {
                        my $value = $allmetrics{$name, "single"}->{$key} / $allmetrics{$name, $type}->{$key};
                        $ratios{$name, $key, $type} = sprintf("%.2f", $value);
                    }
                #to get ratio vs default
                } else {
                    if ($allmetrics{$name, $type}->{$key} == 0) {
                        my $value = 0;
                        $ratios{$name, $key, $type} = sprintf("%.2f", $value);
                    } else {
                        my $value = $allmetrics{$name, $type}->{$key} / $allmetrics{$name, "single"}->{$key};
                        $ratios{$name, $key, $type} = sprintf("%.2f", $value);
                    }
                }
            }
        }
    }
}

my %areaDelay;
# calculate Area-Delay Product
foreach my $name (@benchmarks) {
    foreach my $type (@types) {
        #$allmetrics{$name, $type}->{$areaDelay} = $allmetrics{$name, $type}->{"time"} * $allmetrics{$name, $type}->{"LEs"};
        my $value = $allmetrics{$name, $type}->{"time"} * $allmetrics{$name, $type}->{"LEs"};
        print "$name $type area delay = $value\n";
        $areaDelay{$name, $type} = $value;
    }
}

my $ratioAreaDelay;
foreach my $name (@benchmarks) {
    foreach my $type (@types) {
        if ($type eq "single" ) {
            $ratioAreaDelay{$name, $type} = 1;
        } else {
            if ($areaDelay{$name, $type} == 0 or $areaDelay{$name, "single"} == 0) {
                my $value = 0;
                $ratioAreaDelay{$name, $type} = sprintf("%.2f", $value);
            } else {
                my $value = $areaDelay{$name, $type} / $areaDelay{$name, "single"};
                $ratioAreaDelay{$name, $type} = sprintf("%.2f", $value);
            }
        }
    }
}


#foreach my $key (@metric_keys) {
#    if (defined $performance_key && $key eq $performance_key) {
#        my $value = $geomean{$key};
#        if (defined $metric_units{$key}) {
#            $value .= " $metric_units{$key}";
#        }
#        print "RESULT ".$prefix."all: ".$prefix."geomean= $value\n";
#        print "RESULT ".$prefix."geomean: $prefix$key= $value\n";
#    }
#}

open(CSVFile, '>benchmark.csv') || die "Error: $!\n";
print CSVFile "name ";
#foreach my $type (@types) {
#    foreach my $key (@metric_keys) {
#        print CSVFile "$type $key ";
#    }
#}
print CSVFile "type ";
print CSVFile "result ";
foreach my $key (@metric_keys) {
    print CSVFile "$key ";
}

foreach my $key (@metric_keys) {
    if (($key eq "time") || ($key eq "cycles")) {
        print CSVFile "SpeedUp($key) ";
    } else {
        print CSVFile "Ratio($key) ";
    }
}
print CSVFile "Area-delay Ratio(Area-Delay)";
#print "SpeedUp(time) SpeedUp(cycles) Ratio(Fmax) Ratio(LEs) Ratio{AreaDelay} Ratio(Regs) Ratio(Comb) Ratio(Mult) Ratio(Membits) Ratio(LogicUtil)";

print CSVFile "\n";

foreach my $name (@benchmarks) {
#    print CSVFile "$name ";
    foreach my $type (@types) {
        print CSVFile "$name $type ";
        #print result
        print CSVFile $results{$name, $type}." ";
        #print all metrics
        foreach my $key (@metric_keys) {
            print CSVFile $allmetrics{$name, $type}->{$key}." ";
        }
        #print the ratios
        foreach my $key (@metric_keys) {
            print CSVFile $ratios{$name, $key, $type}." ";
        }
        #print area delay product
        print CSVFile $areaDelay{$name, $type}." ";
        print CSVFile $ratioAreaDelay{$name, $type}." ";
        
        print CSVFile "\n";
    }
    print CSVFile "\n";
}
close(CSVFile);

#print "Latex table (note: for Stratix LEs=ALMs, comb=ALUTs)\n";
#printf "%-20s", "benchmark";
#foreach my $key (@metric_keys) {
#    printf " & %-10s", $key;
#}
#print " \\\\\n";

#foreach my $name (@benchmarks) {
#    printf "%-20s", $name;
#    foreach my $key (@metric_keys) {
#        printf " & %-10s", $allmetrics{$name}->{$key};
#    }
#    print " \\\\\n";
#}
#print "\n";


#print Dumper(\%allmetrics);
#print Dumper(\%geomean);
foreach my $type (@types) {
    print "$type"
#    print "\nCycle geomean: $geomean{$type, cycles}\nFmax geomean: $geomean{$type, Fmax}\nLatency geomean: $geomean{$type, time}\n";
}

print "\nContents of benchmark.csv:\n\n";
system("cat benchmark.csv");


# fail if quartus warnings were seen
#my $fail = 0;
#print "Checking for Quartus Warnings\n";
#foreach my $key (%quartusWarnings) {
#    if ($quartusWarnings{$key}) {
#        print "FAIL: $key\n";
#        print "Quartus Warnings:\n$quartusWarnings{$key}\n";
#        $fail = 1;
#    }
#}

#die "Found Quartus warnings!\n" if ($fail);


sub round {
    my($number) = shift;
    return int($number + .5);
}

# parse the .rpt files for warnings
sub checkForWarnings {
    my $warnings = '';

    # synthesis warnings:
    $warnings .= qx/grep -i "Inferred latch for" top.map.rpt/;
    $warnings .= qx/grep -i "truncated value with size" top.map.rpt/;
    $warnings .= qx/grep -i "assigned a value but never read" top.map.rpt/;

    # timing analysis warnings:
    $warnings .= qx/grep -i "Found combinational loop" top.sta.rpt/;
    # timing requirement not met:
    #$warnings .= qx/grep -i "Critical Warning" top.sta.rpt/;

    # circuit got optimized away
    $warnings .= qx/grep -i "Logic utilization" top.fit.rpt|grep "; 0 %"/;
    $warnings .= qx/grep -i "Total logic elements : 0 " top.fit.summary/;

    return $warnings;
}

sub parse {
    my $name = shift;
    my $type = shift;

    print STDERR "Can't find modelsim 'transcript' for $name\n" unless (-f "transcript");
    my $transcript = qx/grep "# counter =" transcript | tail -n 1/;

    my $fmaxAll;
    if ($stratix) {
        # Stratix IV
        $fmaxAll = qx/grep --after-context=15 -i "Slow 900mV 85C Model Fmax Summary" tiger_top.sta.rpt|grep MHz/;
    } else {
        # Cyclone II
        $fmaxAll = qx/grep --after-context=6 -i "Slow Model Fmax Summary" top.sta.rpt|grep MHz/;
    }

    my $resources = qx/cat tiger_top.fit.summary/;

    # debugging
    if (0) {
        print $transcript;
        print $fmaxAll;
        print $resources;
    }

    my %metrics;

    #check if pass or fail
    my $result = qx/grep "PASS" transcript | tail -n 1/;
    if ($result =~ /PASS/) {
        $result = "PASS";
    }
    else {
        $result = "FAIL";
    }  

    $results{$name, $type} = $result;

    print "\n$name $type\n"; 
    print "------------\n"; 
    print "$result\n"; 
    if ($transcript =~ /counter = +(\d+)/) {
        $metrics{cycles} = $1;
    } elsif ($transcript =~ /Time: (\d+) ns/) {
        my $ns_per_cycle = 20;
        $metrics{cycles} = round($1/$ns_per_cycle);
    } else {
        $metrics{cycles} = 0;
    }


    $metrics{Fmax} = 0;
    my $counter = 0;
#    $counter++ while ($fmaxAll =~ /([\.\d]+) MHz\s*;\s*(clk|clk\[0\]|de2_inst\|pll_inst\|altpll_component\|pll\|clk\[0\])\s+/g);
    $counter++ while ($fmaxAll =~ /([\.\d]+) MHz\s*;\s*(tiger_sopc\|the_ddr2\|ddr2_controller_phy_inst\|ddr2_phy_inst\|ddr2_phy_alt_mem_phy_inst\|clk\|half_rate.pll\|altpll_component\|auto_generated\|pll1\|clk\[0\])\s+/g);
    if ($counter > 1) {
        die "Found more than one Fmax!\nParsing:\n$fmaxAll";
    } elsif ($counter == 1) {
        $metrics{Fmax} = $1;
        print "Fmax: $metrics{Fmax} MHz\n";
    } else {
        #print "Fmax: N/A\n";
        print "Fmax: 0\n";
    }

    my $delay=0;
    print "Latency: $metrics{cycles} cycles\n";
    $metrics{'time'} = 0;
    if ($metrics{Fmax} > 0) {
        $metrics{'time'} = round($metrics{cycles} / $metrics{Fmax} );
        $delay = round($metrics{cycles} / $metrics{Fmax} );
        print "Time: $metrics{time} us\n";
    } else {
        $metrics{Fmax} = "0";
        print "Time: 0\n";
        #$metrics{Fmax} = "N/A";
        #print "Time: N/A\n";
    }

    if ($resources =~ /(Family.*?)Total (GXB|PLL)/s) {
        print $1;
    }


    $resources =~ /Total registers : ([\d,]+)/;
    $metrics{regs} = $1;

    $metrics{LEs} = 0;

    # logic utilization metric meaningless for CycloneII
    $metrics{logicUtil} = 0;

    my $area=0;
    # for stratix LE = ALM, comb = ALUT
    if ($stratix) {
        $resources =~ /ALUTs : ([\d,]+)/;
        $metrics{comb} = $1;

        my $alm_raw = qx/grep "ALMs:" tiger_top.fit.rpt/;
        if ($alm_raw =~ /ALMs:\s+partially or completely used\s+; ([\d,]+)/) {
            $metrics{LEs} = $1;
            $area = $1;
	    print "ALMs: $1";
        } 

        my $lu_raw = qx/grep "Logic utilization" tiger_top.fit.rpt/;
        if ($lu_raw =~ /Logic utilization\s+; ([\d,]+) \//) {
            $metrics{logicUtil} = $1;
        }

        $resources =~ /DSP block 18-bit elements : ([\d,]+)/;
        $metrics{mults} = $1;
    } else {
        # cyclone
        $resources =~ /Total combinational functions : ([\d,]+)/;
        $metrics{comb} = $1;

        $resources =~ /Total logic elements : ([\d,]+)/;
        $metrics{LEs} = $1;

        $resources =~ /Embedded Multiplier 9-bit elements : ([\d,]+)/;
        $metrics{mults} = $1;
    }


    $resources =~ /memory bits : ([\d,]+)/;
    $metrics{membits} = $1;

    #calculate Area-Delay product
  #  $metrics{areaDelay} = $area * $delay;
    print "\n";


    foreach my $key (@metric_keys) {
        $metrics{$key} =~ s/,//g;

        # Performance graphs
        if (defined $performance_key && $key eq $performance_key) {
            my $value = $metrics{$key};
            if (defined $metric_units{$key}) {
                $value .= " $metric_units{$key}";
            }
           # can't have a '/' in name:
            my $stripped = $name;
            $stripped =~ s/chstone\///;
            print "RESULT ".$prefix."all: $prefix$stripped= $value\n";
            print "RESULT $prefix$stripped: $prefix$key= $value\n";
        }
    }

    return \%metrics;
}
