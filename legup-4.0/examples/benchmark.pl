#!/usr/bin/perl
#
# USAGE: benchmark.pl --family <family> --key <key> --prefix <prefix> --dirs <folder1 folder2 ...>
#

use warnings;
use strict;
use Cwd;
use Data::Dumper;
use Getopt::Long;

my $dirs = "chstone/adpcm  chstone/aes  chstone/blowfish  chstone/dfadd ".
"chstone/dfdiv  chstone/dfmul  chstone/dfsin  chstone/gsm  chstone/jpeg ".
"chstone/mips  chstone/motion  chstone/sha dhrystone";

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

my $quartus = 0;

# -b backup_directory
# copies all of the following files for the @benchmarks:
# watch out: adds dg.exp
my $fileTypes = "*.v transcript *.rpt *.summary *.c *.h config.tcl Makefile dg.exp softfloat-macros softfloat-specialize";
my $backupDir = '';

# Note, to run this script from within a 'backup' directory run:
#       benchmarks.pl -b backup
#       cd backup
#       ../benchmarks.pl

GetOptions ("family=s" => \$family,
            "key=s" => \$performance_key,
            "prefix=s" => \$prefix,
            "dirs=s" => \$dirs,
            "q" => \$quartus,
            "b=s" => \$backupDir,
           ) || die;

my @benchmarks = split /\s+/, $dirs;

$family = lc $family;
my $stratix = 0;
$stratix = 1 if ($family =~ /stratix/i);
if (!$stratix) {
    die ("Family '$family' must be Cyclone or Stratix\n") unless ($family =~ /cyclone/i);
}

if ($backupDir ne '') {

    my $pwd = &Cwd::cwd();

    foreach my $origDir (@benchmarks) {
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
    system("cd $backupDir && perl -pi -e 's|LEVEL = ..|LEVEL = ../..|' `find . -name Makefile`");
    exit;
}



my $pwd = &Cwd::cwd();

my %metric_units = (
    'time' => 'us',
    'Fmax' => 'MHz',
);

my %allmetrics;
my %quartusWarnings;

my @metric_keys = ();
#push @metric_keys, qw(II);
push @metric_keys, qw(time cycles Fmax LEs regs comb mults membits logicUtil II);
if ($stratix) {
    push @metric_keys, qw(M9K M144K);
}

foreach my $name (@benchmarks) {
    chdir $name or die "$!";

    if ($quartus) {
        #system("make");
        #system("make v");
        system("make p");
        system("make f");
    }

    $quartusWarnings{$name} = checkForWarnings($name);

    $allmetrics{$name} = parse($name);

    chdir $pwd or die "$!";
}

# calculate geomean
my %geomean;
foreach my $key (@metric_keys) {
    $geomean{$key} = 1;
}
my $count = 0;
foreach my $name (@benchmarks) {
    if ($allmetrics{$name}->{Fmax} ne "N/A") {
        $count++;
        foreach my $key (@metric_keys) {
            my $value = $allmetrics{$name}->{$key};
            # for 0 entries just assume 1. otherwise geomean will just equal 0.
            if ($value != 0) {
                $geomean{$key} *= $value;
            }
        }
    }
}
$count = 1 if ($count == 0); # avoid division by zero
foreach my $key (@metric_keys) {
    $geomean{$key} **= 1.0/$count;
}

foreach my $key (@metric_keys) {
    if (defined $performance_key && $key eq $performance_key) {
        my $value = $geomean{$key};
        if (defined $metric_units{$key}) {
            $value .= " $metric_units{$key}";
        }
        print "RESULT ".$prefix."all: ".$prefix."geomean= $value\n";
        print "RESULT ".$prefix."geomean: $prefix$key= $value\n";
    }
}


open(CSVFile, '>benchmark.csv') || die "Error: $!\n";
print CSVFile "name ";
foreach my $key (@metric_keys) {
    print CSVFile "$key ";
}
print CSVFile "\n";

foreach my $name (@benchmarks) {
    print CSVFile "$name ";
    foreach my $key (@metric_keys) {
        print CSVFile $allmetrics{$name}->{$key}." ";
    }
    print CSVFile "\n";
}
print CSVFile "geomean ";
foreach my $key (@metric_keys) {
	printf CSVFile "%.2f ", $geomean{$key};
}
print CSVFile "\n";
close(CSVFile);

print "Family: $family\n";
print "Latex table (note: for Stratix LEs=ALMs, comb=ALUTs)\n";
printf "%-20s", "benchmark";
foreach my $key (@metric_keys) {
    printf " & %-10s", $key;
}
print " \\\\\n";

foreach my $name (@benchmarks) {
    printf "%-20s", $name;
    foreach my $key (@metric_keys) {
        printf " & %-10s", $allmetrics{$name}->{$key};
    }
    print " \\\\\n";
}
printf "%-20s", "geomean";
foreach my $key (@metric_keys) {
	printf " & %-10.2f", $geomean{$key};
}
print " \\\\\n";
print "\n";


#print Dumper(\%allmetrics);
#print Dumper(\%geomean);

print "\nContents of benchmark.csv:\n\n";
system("cat benchmark.csv");

# fail if quartus warnings were seen
my $fail = 0;
print "Checking for Quartus Warnings\n";
foreach my $key (%quartusWarnings) {
    if ($quartusWarnings{$key}) {
        print "FAIL: $key\n";
        print "Quartus Warnings:\n$quartusWarnings{$key}\n";
        $fail = 1;
    }
}

die "Found Quartus warnings!\n" if ($fail);


# parse the .rpt files for warnings
sub checkForWarnings {
    my $warnings = '';

    # synthesis warnings:
    $warnings .= qx/grep -i "Inferred latch for" top.map.rpt/;

    # TODO - fix local RAMs and add this back in
    #$warnings .= qx/grep -i "truncated value with size" top.map.rpt/;

    # TODO - fix local RAMs and add this back in
    #$warnings .= qx/grep -i "assigned a value but never read" top.map.rpt/;

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

    my %metrics;

    print STDERR "Can't find modelsim 'transcript' for $name\n" unless (-f "transcript");
    my $transcript = qx/grep "Time:" transcript | tail -n 1/;

    $metrics{II} = 0;
    if (-f "pipelining.legup.rpt") {
        my $pipelineII = qx/grep II pipelining.legup.rpt| tail -n 1/;
        if ($pipelineII =~ /II = (\d+)/) {
            $metrics{II} = $1;
        }
    }

    my $fmaxAll;
    if ($stratix) {
        # Stratix IV
        $fmaxAll = qx/grep --after-context=6 -i "Slow 900mV 0C Model Fmax Summary" top.sta.rpt|grep MHz/;
    } else {
        # Cyclone II
        $fmaxAll = qx/grep --after-context=6 -i "Slow Model Fmax Summary" top.sta.rpt|grep MHz/;
    }

    my $lines = qx/wc -l *.v|grep -v tb|grep -v total/;
    my $resources = qx/cat top.fit.summary/;

    # debugging
    if (0) {
        print $transcript;
        print $fmaxAll;
        print $lines;
        print $resources;
    }

    print "$name\n"; 
    print "------------\n"; 
    print "II: $metrics{II}\n" if (exists $metrics{II});

    if ($transcript =~ /counter = (\d+)/) {
        $metrics{cycles} = $1;
    } elsif ($transcript =~ /Time: (\d+) ns/) {
        my $ns_per_cycle = 20;
        $metrics{cycles} = sprintf "%.f", $1/$ns_per_cycle;
    } else {
        $metrics{cycles} = 0;
    }

    $metrics{Fmax} = 0;
    my $counter = 0;
    $counter++ while ($fmaxAll =~ /([\.\d]+) MHz\s*;\s*(clk|CLOCK_50|OSC_50_BANK2|clk\[0\]|de2_inst\|pll_inst\|altpll_component\|pll\|clk\[0\])\s+/g);
    if ($counter > 1) {
        die "Found more than one Fmax!\nParsing:\n$fmaxAll";
    } elsif ($counter == 1) {
        $metrics{Fmax} = sprintf "%.f", $1;
        print "Fmax: $metrics{Fmax} MHz\n";
    } else {
        print "Fmax: N/A\n";
    }

    print "Latency: $metrics{cycles} cycles\n";
    $metrics{'time'} = 0;
    if ($metrics{Fmax} > 0) {
        $metrics{'time'} = sprintf "%.1f", $metrics{cycles} / $metrics{Fmax};
        print "Time: $metrics{time} us\n";
    } else {
        $metrics{Fmax} = "N/A";
        print "Time: N/A\n";
    }

    if ($lines =~ /(\d+)/) {
        print "Verilog: $1 LOC\n";
    } else {
        print "Verilog: 0 LOC\n";
    }

    if ($resources =~ /(Family.*?)Total (GXB|PLL)/s) {
        my $device_info = $1;
        print $device_info;
        $device_info =~ /Family\s*:\s*(.*)\s+/;
        my $detected_family = lc $1;
        $detected_family =~ s/\s+//;
        if ($family ne $detected_family) {
            die ("Family mismatch for benchmark $name!\n".
                "\tUser specified family $family != $detected_family\n".
                "\tPlease modify benchmark.pl or use: benchmark.pl --family <family>\n");
        }
    }


    $resources =~ /Total registers : ([\d,]+)/;
    $metrics{regs} = $1;

    $metrics{LEs} = 0;

    # logic utilization metric meaningless for CycloneII
    $metrics{logicUtil} = 0;

    # for stratix LE = ALM, comb = ALUT
    if ($stratix) {
        $resources =~ /ALUTs : ([\d,]+)/;
        $metrics{comb} = $1;

        my $alm_raw = qx/grep "ALMs:" top.fit.rpt/;
        if ($alm_raw =~ /ALMs:\s+partially or completely used\s+; ([\d,]+)/) {
            $metrics{LEs} = $1;
        } 

        my $lu_raw = qx/grep "Logic utilization" top.fit.rpt/;
        if ($lu_raw =~ /Logic utilization\s+; ([\d,]+) \//) {
            $metrics{logicUtil} = $1;
        }

        $resources =~ /DSP block 18-bit elements : ([\d,]+)/;
        $metrics{mults} = $1;

        my $fit_rpt = qx/cat top.fit.rpt/;
        $metrics{M9K} = 0;
        if ($fit_rpt =~ /; M9K blocks\s+; ([\d,]+)/) {
            $metrics{M9K} = $1;
        }
        $metrics{M144K} = 0;
        if ($fit_rpt =~ /; M144K blocks\s+; ([\d,]+)/) {
            $metrics{M144K} = $1;
        }

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

    print "\n";

    foreach my $key (@metric_keys) {
        die "Cannot find key: $key\n" unless (exists $metrics{$key});
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

    my $keys_expected = scalar(@metric_keys);
    my $keys_found = scalar(keys %metrics);
    die("Expected: ".Dumper(\@metric_keys)."Actual:".Dumper(keys %metrics)) unless ($keys_expected == $keys_found);


    return \%metrics;
}
