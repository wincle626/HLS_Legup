#!/usr/bin/perl
#
# USAGE: benchmark.pl --family <family> --key <key> --prefix <prefix>
#

use warnings;
use strict;
use Cwd;
use Data::Dumper;
use Getopt::Long;

# Produces latex tables from scinet runs

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

#my $folder = "kernels-orig/examples/multipump";
#my $folder = "kernels-all-long/examples/multipump";
#my $folder = "kernels-all-5h/examples/multipump";
my $folder = $ARGV[0]."/examples/multipump";
#my @benchmarks = qw|alphablend gaussblur idct mandelbrot matrixmultiply sobel|;
#my @benchmarks = qw|alphablend sobel matrixmultiply gaussblur idct|;
my @benchmarks = qw|
alphablend alphablend_rs alphablend_mp
sobel sobel_rs sobel_mp
4matrixmult 4matrixmult_rs 4matrixmult_mp
gaussblur gaussblur_rs gaussblur_mp
idct idct_rs idct_mp
mandelbrot mandelbrot_rs mandelbrot_mp
|;
#matrixmultiply matrixmultiply_rs matrixmultiply_mp
#my @benchmarks = qw|chstone/adpcm chstone/dfdiv chstone/dfmul chstone/dfsin chstone/gsm chstone/jpeg chstone/mips dhrystone|;
#die unless scalar(@benchmarks) == 8;

my $pwd = &Cwd::cwd();

my @metric_keys = qw(time cycles Fmax LEs regs comb mults membits logicUtil);


my %metric_units = (
    'time' => 'us',
    'Fmax' => 'MHz',
);

my %allmetrics;
my %quartusWarnings;

foreach my $name (@benchmarks) {
    #chdir $name or die "$!";
    my $dest = "$folder/$name";
    $dest =~ s/4matrixmult/matrixmultiply/;
    #chdir $dest or next;
    chdir $dest or die "Failed to open $folder/$name: $!";

    #$quartusWarnings{$name} = checkForWarnings($name);

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
    $geomean{$key} = sprintf "%.2f", $geomean{$key};
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
close(CSVFile);

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
print "\n";


open(my $L, '>table.tex') || die "Error: $!\n";

print $L '\begin{table*}[htbp]'."\n";
print $L '\centering'."\n";
print $L '\caption{Area results (TRS: Traditional Resource Sharing, MP: Multi-Pumping)}'."\n";
print $L '\vskip -0.01in'."\n";
print $L '\begin{tabular}{|l|c|c|c|c|c|c|c|c|c|}'."\n";
print $L '\hline'."\n";
print $L '& \multicolumn{ 3}{c|}{\textbf{DSPs}} & \multicolumn{ 3}{c|}{\textbf{Registers}} & \multicolumn{ 3}{c|}{\textbf{ALUTs}} \\\\ \hline'."\n";
print $L '\textbf{Benchmark} & \multicolumn{1}{c|}{\textbf{Orig}} & \multicolumn{1}{c|}{\textbf{TRS}} & \multicolumn{1}{c|}{\textbf{MP}} & \multicolumn{1}{c|}{\textbf{Orig}} & \multicolumn{1}{c|}{\textbf{TRS}} & \multicolumn{1}{c|}{\textbf{MP}} & \multicolumn{1}{c|}{\textbf{Orig}} & \multicolumn{1}{c|}{\textbf{TRS}} & \multicolumn{1}{c|}{\textbf{MP}} \\\\ \hline'."\n";

#print 'alphablend & 8 & 4 & 4 & 4748 & 5773 & 4918 & 8648 & 11449 & 8750 \\ \hline'."\n";
#print 'sobel & 8 & 4 & 4 & 26503 & 26624 & 26531 & 24540 & 25123 & 24671 \\ \hline'."\n";
#print '4matrixmult & 16 & 8 & 8 & 10655 & 15437 & 10810 & 10804 & 28164 & 11081 \\ \hline'."\n";
#print 'gauss\_blur & 24 & 12 & 12 & 10663 & 10810 & 10754 & 10588 & 10882 & 10825 \\ \hline'."\n";
#print 'idct & 40 & 20 & 20 & 41421 & 39862 & 41713 & 31441 & 32754 & 33231 \\ \hline'."\n";
#print 'mandelbrot & 144 & 72 & 72 & 32123 & 31589 & 34173 & 33615 & 33080 & 34173 \\ \hline'."\n";

my @table_names = (
['alphablend', 'alphablend_rs', 'alphablend_mp', 'alphablend', 'alphablend_rs', 'alphablend_mp', 'alphablend', 'alphablend_rs', 'alphablend_mp'],
['sobel', 'sobel_rs', 'sobel_mp', 'sobel', 'sobel_rs', 'sobel_mp', 'sobel', 'sobel_rs', 'sobel_mp'],
['4matrixmult', '4matrixmult_rs', '4matrixmult_mp', '4matrixmult', '4matrixmult_rs', '4matrixmult_mp', '4matrixmult', '4matrixmult_rs', '4matrixmult_mp'],
['gaussblur', 'gaussblur_rs', 'gaussblur_mp', 'gaussblur', 'gaussblur_rs', 'gaussblur_mp', 'gaussblur', 'gaussblur_rs', 'gaussblur_mp'],
['idct', 'idct_rs', 'idct_mp', 'idct', 'idct_rs', 'idct_mp', 'idct', 'idct_rs', 'idct_mp'],
['mandelbrot', 'mandelbrot_rs', 'mandelbrot_mp', 'mandelbrot', 'mandelbrot_rs', 'mandelbrot_mp', 'mandelbrot', 'mandelbrot_rs', 'mandelbrot_mp'],
);
my @table_keys = (
['mults', 'mults', 'mults', 'regs', 'regs', 'regs', 'comb', 'comb', 'comb'],
['mults', 'mults', 'mults', 'regs', 'regs', 'regs', 'comb', 'comb', 'comb'],
['mults', 'mults', 'mults', 'regs', 'regs', 'regs', 'comb', 'comb', 'comb'],
['mults', 'mults', 'mults', 'regs', 'regs', 'regs', 'comb', 'comb', 'comb'],
['mults', 'mults', 'mults', 'regs', 'regs', 'regs', 'comb', 'comb', 'comb'],
['mults', 'mults', 'mults', 'regs', 'regs', 'regs', 'comb', 'comb', 'comb'],
);


print_table($L, \@table_names, \@table_keys);

print $L '\end{tabular}'."\n";
print $L '\label{tbl:area}'."\n";
print $L '\end{table*}'."\n";
print $L ''."\n";
print $L '\begin{table*}[htbp]'."\n";
print $L '\centering'."\n";
print $L '\caption{Speed performance results (TRS: Traditional Resource Sharing, MP: Multi-Pumping)}'."\n";
print $L '\vskip -0.01in'."\n";
print $L '\begin{tabular}{|l|c|c|c|c|c|c|c|c|c|}'."\n";
print $L '\hline'."\n";
print $L '\multicolumn{1}{|c|}{} & \multicolumn{ 3}{c|}{\textbf{Cycles}} & \multicolumn{ 3}{c|}{\textbf{Fmax (MHz)}} & \multicolumn{ 3}{c|}{\textbf{Time ($\mu$s)}} \\\\ \hline'."\n";
print $L '\textbf{Benchmark} & \textbf{Orig} & \textbf{TRS} & \textbf{MP} & \textbf{Orig} & \textbf{TRS} & \textbf{MP} & \textbf{Orig} & \textbf{TRS} & \textbf{MP} \\\\ \hline'."\n";

#print 'alphablend & 1111 & 2111 & 1151 & 207 & 187 & 179 & 5.4 & 11.3 & 6.4 \\ \hline'."\n";
#print 'sobel & 45141 & 65813 & 46229 & 152 & 164 & 168 & 297.0 & 401.3 & 275.2 \\ \hline'."\n";
#print '4matrixmult & 8451 & 19651 & 8651 & 157 & 139 & 235 & 53.8 & 141.4 & 36.8 \\ \hline'."\n";
#print 'gauss\_blur & 25487 & 45615 & 26575 & 156 & 164 & 159 & 163.4 & 278.1 & 167.1 \\ \hline'."\n";
#print 'idct & 6736 & 11136 & 7436 & 147 & 132 & 98 & 45.8 & 84.4 & 75.9 \\ \hline'."\n";
#print 'mandelbrot & 1899 & 3339 & 2091 & 129 & 123 & 109 & 14.7 & 27.1 & 19.2 \\ \hline'."\n";

my @table_names = (
['alphablend', 'alphablend_rs', 'alphablend_mp', 'alphablend', 'alphablend_rs', 'alphablend_mp', 'alphablend', 'alphablend_rs', 'alphablend_mp'],
['sobel', 'sobel_rs', 'sobel_mp', 'sobel', 'sobel_rs', 'sobel_mp', 'sobel', 'sobel_rs', 'sobel_mp'],
['4matrixmult', '4matrixmult_rs', '4matrixmult_mp', '4matrixmult', '4matrixmult_rs', '4matrixmult_mp', '4matrixmult', '4matrixmult_rs', '4matrixmult_mp'],
['gaussblur', 'gaussblur_rs', 'gaussblur_mp', 'gaussblur', 'gaussblur_rs', 'gaussblur_mp', 'gaussblur', 'gaussblur_rs', 'gaussblur_mp'],
['idct', 'idct_rs', 'idct_mp', 'idct', 'idct_rs', 'idct_mp', 'idct', 'idct_rs', 'idct_mp'],
['mandelbrot', 'mandelbrot_rs', 'mandelbrot_mp', 'mandelbrot', 'mandelbrot_rs', 'mandelbrot_mp', 'mandelbrot', 'mandelbrot_rs', 'mandelbrot_mp'],
);
my @table_keys = (
['cycles', 'cycles', 'cycles', 'Fmax', 'Fmax', 'Fmax', 'time', 'time', 'time'],
['cycles', 'cycles', 'cycles', 'Fmax', 'Fmax', 'Fmax', 'time', 'time', 'time'],
['cycles', 'cycles', 'cycles', 'Fmax', 'Fmax', 'Fmax', 'time', 'time', 'time'],
['cycles', 'cycles', 'cycles', 'Fmax', 'Fmax', 'Fmax', 'time', 'time', 'time'],
['cycles', 'cycles', 'cycles', 'Fmax', 'Fmax', 'Fmax', 'time', 'time', 'time'],
['cycles', 'cycles', 'cycles', 'Fmax', 'Fmax', 'Fmax', 'time', 'time', 'time'],
);



print_table($L, \@table_names, \@table_keys);

#print '\textbf{Geomean} & \textbf{7190} & \textbf{12910} & \textbf{7584} & \textbf{156} & \textbf{150} & \textbf{151} & \textbf{46.0} & \textbf{86.1} & \textbf{50.1} \\ \hline'."\n";
#print '\textbf{Ratio} & \textbf{1} & \textbf{1.8} & \textbf{1.05} & \textbf{1} & \textbf{0.96} & \textbf{0.97} & \textbf{1} & \textbf{1.87} & \textbf{1.09} \\ \hline'."\n";
print $L '\end{tabular}'."\n";
print $L '\label{tbl:speed}'."\n";
print $L '\end{table*}'."\n";






#print Dumper(\%allmetrics);
#print Dumper(\%geomean);
print "\nCycle geomean: $geomean{cycles}\nFmax geomean: $geomean{Fmax}\nLatency geomean: $geomean{time}\n";

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

print "\nContents of table.tex:\n\n";
system("cat table.tex");


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

    return $warnings;
}

sub parse {
    my $name = shift;

    print STDERR "Can't find modelsim 'transcript' for $name\n" unless (-f "transcript");
    my $transcript = qx/grep "Time:" transcript | tail -n 1/;

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

    my %metrics;

    print "$name\n"; 
    print "------------\n"; 
    if ($transcript =~ /counter = (\d+)/) {
        $metrics{cycles} = $1;
    } elsif ($transcript =~ /Time: (\d+) ns/) {
        my $ns_per_cycle = 20;
        $metrics{cycles} = round($1/$ns_per_cycle);
    } else {
        $metrics{cycles} = 0;
    }

    #$metrics{Fmax} = 0;
    #my $counter = 0;
    #$counter++ while ($fmaxAll =~ /([\.\d]+) MHz.*(clk|clk\[0\])\s+/g);
    #if ($counter > 1) {
    #    die "Found more than one Fmax!\nParsing:\n$fmaxAll";
    #} elsif ($counter == 1) {
    #    $metrics{Fmax} = $1;
    #    print "Fmax: $metrics{Fmax} MHz\n";
    #} else {
    #    print "Fmax: N/A\n";
    #}

    my $fmax = 0;
    my $counter = 0;
    $counter++ while ($fmaxAll =~ /([\.\d]+) MHz\s*;\s*(de2_inst\|pll_inst\|altpll_component\|pll\|clk\[0\])\s+/g);
    if ($counter > 1) {
        die "Found more than one Fmax!\nParsing:\n$fmaxAll";
    } elsif ($counter == 1) {
        $fmax = $1;
        print "Fmax: $fmax MHz\n";
    } else {
        print "Fmax: N/A\n";
    }

    my $fmax2x = undef;
    $counter = 0;
    $counter++ while ($fmaxAll =~ /([\.\d]+) MHz\s*;\s*(de2_inst\|pll_inst\|altpll_component\|pll\|clk\[1\])\s+/g);
    if ($counter > 1) {
        die "Found more than one Fmax!\nParsing:\n$fmaxAll";
    } elsif ($counter == 1) {
        $fmax2x = $1;
        print "Fmax2x: $fmax2x MHz\n";
    } else {
        print "Fmax2x: N/A\n";
    }

    if (defined $fmax2x) {
        $fmax = min($fmax, $fmax2x/2);
    }
    $metrics{Fmax} = $fmax;




    print "Latency: $metrics{cycles} cycles\n";
    $metrics{'time'} = 0;
    if ($metrics{Fmax} > 0) {
        $metrics{'time'} = sprintf "%.2f", $metrics{cycles} / $metrics{Fmax};
        print "Latency: $metrics{time} us\n";
    } else {
        $metrics{Fmax} = "N/A";
        print "Latency: N/A\n";
    }

    if ($lines =~ /(\d+)/) {
        print "Verilog: $1 LOC\n";
    } else {
        print "Verilog: 0 LOC\n";
    }

    if ($resources =~ /(Family.*?)Total (GXB|PLL)/s) {
        print $1;
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

sub min {
    my $a = shift;
    my $b = shift;
    return $a if ($a < $b);
    return $b;
}

sub print_table {
    my $L = shift;
    my @table_names = @{shift(@_)};
    my @table_keys = @{shift(@_)};

    # calculate geomean
    my %col_geomean;
    foreach my $i (0..(scalar(@table_names)-1)) {
        next unless(defined $table_names[$i]);
        my @row = @{$table_names[$i]};
        foreach my $j (0..(scalar(@row)-1)) {
            $col_geomean{$j} = 1;
        }
    }

    my $rows = 0;
    my $cols = 0;
    foreach my $i (0..(scalar(@table_names)-1)) {
        next unless(defined $table_names[$i]);
        $rows++;
        my @row = @{$table_names[$i]};
        printf $L "%-20s", $table_names[$i]->[0];
        $cols = scalar(@row);
        foreach my $j (0..(scalar(@row)-1)) {
            my $name = $table_names[$i]->[$j];
            if (!defined $name) {
                printf $L " & %-10s", '-';
                next;
            }
            my $key = $table_keys[$i]->[$j];
            my $val = $allmetrics{$name}->{$key};
            if (defined $val) {
                $col_geomean{$j} *= $val;
                $val = sprintf "%.1f", $val if ($key =~ /time/);
                $val = sprintf "%.f", $val if ($key =~ /Fmax/);
            } else {
                $val = '-';
            }
            # add commas
            $val =~ s/(^[-+]?\d+?(?=(?>(?:\d{3})+)(?!\d))|\G\d{3}(?=\d))/$1,/g;
            printf $L " & %-10s", $val;
        }
        print $L " \\\\\n";
        print $L '\hline'."\n";
    }

    print $L '\hline'."\n";

    #print '\textbf{Geomean} & \textbf{23} & \textbf{11} & \textbf{11} & \textbf{16339} & \textbf{17845} & \textbf{16691} & \textbf{17174} & \textbf{21380} & \textbf{17569} \\ \hline'."\n";

    print $L '\textbf{Geomean} ';
    $rows = 1 if ($rows == 0); # avoid division by zero
    my $ratioStr = '\textbf{Ratio} ';
    my %ratio = ();
    foreach my $i (0..$cols-1) {
        $col_geomean{$i} **= 1.0/$rows;
        $ratio{$i} = $col_geomean{$i} / $col_geomean{$i-$i%3};
    }
    foreach my $i (0..$cols-1) {
        my $key = $table_keys[0]->[$i];
        if ($key =~ /time/) {
            $col_geomean{$i} = sprintf "%.1f", $col_geomean{$i};
        } else {
            $col_geomean{$i} = sprintf "%.f", $col_geomean{$i} ;
        }
        $col_geomean{$i} =~ s/(^[-+]?\d+?(?=(?>(?:\d{3})+)(?!\d))|\G\d{3}(?=\d))/$1,/g;
        print $L ' & \textbf{'.$col_geomean{$i}.'} ';
        $ratio{$i} = sprintf "%.2f", $ratio{$i};
        $ratio{$i} =~ s/0*$//;
        $ratio{$i} =~ s/\.$//;
        $ratioStr .= ' & \textbf{'.$ratio{$i}.'} ';
    }
    print $L '\\\\ \hline'."\n";
    print $L $ratioStr;
    print $L '\\\\ \hline'."\n";

    #print '\textbf{Ratio} & \textbf{1} & \textbf{0.5} & \textbf{0.5} & \textbf{1} & \textbf{1.09} & \textbf{1.02} & \textbf{1} & \textbf{1.24} & \textbf{1.02} \\ \hline'."\n";
}
