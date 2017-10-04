#!/usr/bin/perl
use warnings;
use strict;

#my @widths = (8, 16, 32, 64);
#my @widths = (64);
#my @widths = (8, 16, 32, 64);
my @widths = reverse (8, 16, 32, 64);

my @pipelines = (0, 1, 2, 3, 4, 5, 6);
#my @pipelines = (0, 1, 2, 3, 4, 5);
#my @pipelines = (6);
#my @pipelines = (4, 5);
#my @pipelines = 6..16;
#my @pipelines = 0..16;
#my @pipelines = (0, 1, 2, 3);
#my @pipelines = (1, 2, 3);
#my @pipelines = (0);
#my @pipelines = (1);

my @signs = ('SIGNED', 'UNSIGNED');
#my @signs = ('UNSIGNED');

#my @seeds = 1..10;
my @seeds = (1);

my $initial = 0;
my $quartus = 0;


foreach my $sign (@signs) {
    foreach my $width (@widths) {
        open(FMAX, ">fmax_$sign\_$width.dat") || die "Error: $!\n";
        open(FMAX2X, ">fmax2x_$sign\_$width.dat") || die "Error: $!\n";
        open(FMAXREAL, ">fmaxreal_$sign\_$width.dat") || die "Error: $!\n";
        open(FMAXREAL2, ">fmaxreal2_$sign\_$width.dat") || die "Error: $!\n";
        open(ALUT, ">alut_$sign\_$width.dat") || die "Error: $!\n";
        open(REG, ">reg_$sign\_$width.dat") || die "Error: $!\n";
        open(ALM, ">alm_$sign\_$width.dat") || die "Error: $!\n";
        open(DSP, ">dsp_$sign\_$width.dat") || die "Error: $!\n";
        foreach my $pipeline (@pipelines) {
            my $fmax_sum = 0;
            my $fmax2x_sum = 0;
            my $fmaxreal_sum = 0;
            my $fmaxreal2_sum = 0;
            my $alut_sum = 0;
            my $reg_sum = 0;
            my $alm_sum = 0;
            my $dsp_sum = 0;
            foreach my $seed (@seeds) {
                my $signlc = lc $sign;
                my $dir = "$signlc/$pipeline/$width/$seed";
                if ($initial) {
                    system ("mkdir -p $dir");
                    system ("cp Makefile.new $dir/Makefile");
                    system ("cp multipump.v $dir");
                    system ("cp top.qsf $dir");
                    system ("cp multipump.sdc $dir");
                }
                chdir $dir;
                if ($initial) {
                    if ($pipeline > 0) {
                        system (q|perl -pi -e 's/\.clock \(\)/\.clock\(clk2x\)/' multipump.v|);
                    }
                    my $fitseed = int(rand(10000000));
                    system ("perl -pi -e 's/SEED \\d+/SEED $fitseed/' top.qsf");
                    system ("perl -pi -e 's/parameter size = 32/parameter size = $width/' multipump.v");
                    system (qq|perl -pi -e 's/parameter sign = "UNSIGNED"/parameter sign = "$sign"/' multipump.v|);
                    system ("perl -pi -e 's/parameter pipeline = 0/parameter pipeline = $pipeline/' multipump.v");
                }
                if ($quartus) {
                    system ("make f");
                }

                my $fmaxAll = qx/grep --after-context=6 -i "Slow 900mV 0C Model Fmax Summary" top.sta.rpt|grep MHz/;
                my $resources = qx/cat top.fit.summary/;

                print "Dir: $dir\n";
                my $fmax = 0;
                my $counter = 0;
                $counter++ while ($fmaxAll =~ /([\.\d]+) MHz\s*;\s*(pll_inst\|altpll_component\|pll\|clk\[0\])\s+/g);
                if ($counter > 1) {
                    die "Found more than one Fmax!\nParsing:\n$fmaxAll";
                } elsif ($counter == 1) {
                    $fmax = $1;
                    print "Fmax: $fmax MHz\n";
                } else {
                    print "Fmax: N/A\n";
                }

                my $fmax2x = 0;
                $counter = 0;
                $counter++ while ($fmaxAll =~ /([\.\d]+) MHz\s*;\s*(pll_inst\|altpll_component\|pll\|clk\[1\])\s+/g);
                if ($counter > 1) {
                    die "Found more than one Fmax!\nParsing:\n$fmaxAll";
                } elsif ($counter == 1) {
                    $fmax2x = $1;
                    print "Fmax2x: $fmax2x MHz\n";
                } else {
                    print "Fmax2x: N/A\n";
                }

                my $realfmax = min($fmax, $fmax2x/2);
                my $realfmax2x = $realfmax*2;

                #print FMAX "$pipeline $realfmax\n";
                #$fmax_sum += $realfmax;
                $fmax_sum += $fmax;

                #print FMAX2X "$pipeline $realfmax2x\n";
                #$fmax2x_sum += $realfmax2x;
                $fmax2x_sum += $fmax2x;

                $fmaxreal_sum += $realfmax;
                $fmaxreal2_sum += $realfmax2x;


                $resources =~ /ALUTs : ([\d,]+)/;
                my $alut = $1;
                #print ALUT "$pipeline $alut\n";
                $alut_sum += $alut;

                $resources =~ /Total registers : ([\d,]+)/;
                my $reg = $1;
                #print REG "$pipeline $reg\n";
                $reg_sum += $reg;

                my $alm = 0;
                my $alm_raw = qx/grep "ALMs:" top.fit.rpt/;
                if ($alm_raw =~ /ALMs:\s+partially or completely used\s+; ([\d,]+)/) {
                    $alm = $1;
                } 
                #print ALM "$pipeline $alm\n";
                $alm_sum += $alm;


                $resources =~ /DSP block 18-bit elements : ([\d,]+)/ || die;
                my $dsp = $1;
                #print DSP "$pipeline $dsp\n";
                $dsp_sum += $dsp;

                if ($resources =~ /(Family.*?)Total (GXB|PLL)/s) {
                    print $1;
                }
                print "\n";

                chdir "../../../..";
            }
            my $count = scalar(@seeds);
            # get the mean over the seed sweep
            print FMAX "$pipeline ".$fmax_sum/$count."\n";
            print FMAX2X "$pipeline ".$fmax2x_sum/$count."\n";
            print FMAXREAL "$pipeline ".$fmaxreal_sum/$count."\n";
            print FMAXREAL2 "$pipeline ".$fmaxreal2_sum/$count."\n";
            print ALUT "$pipeline ".$alut_sum/$count."\n";
            print REG  "$pipeline ".$reg_sum/$count."\n";
            print ALM  "$pipeline ".$alm_sum/$count."\n";
            print DSP  "$pipeline ".$dsp_sum/$count."\n";
        }
        close(FMAX);
        close(FMAX2X);
        close(FMAXREAL);
        close(FMAXREAL2);
        close(ALUT);
        close(REG);
        close(ALM);
        close(DSP);
    }
}

open (GNUPLOT, "|gnuplot");
#open (GNUPLOT, ">multipump.gnuplot");
my $EPS = 0;
my $PS = 1;
my $PNG = 2;
my $type = $EPS;
#my $type = $PS;
#my $type = $PNG;
my $ext;
if ($type == $EPS) {
    $ext = "eps";
    print GNUPLOT "set terminal epslatex monochrome\n";
} elsif ($type == $PS) {
    print GNUPLOT "set terminal postscript dashed\n";
    $ext = "ps";
} elsif ($type == $PNG) {
    $ext = "png";
    #print GNUPLOT "set terminal png\n";
    #print GNUPLOT "set terminal epslatex dashed\n";
    # export GDFONTPATH=/usr/share/fonts/truetype/ttf-bitstream-vera
    print GNUPLOT "set terminal png truecolor nocrop enhanced font Vera 20 size 1024,768 linewidth 3\n";
}
#print GNUPLOT "set terminal png dashed\n";
#print GNUPLOT "set termoption dashed\n";

my %yaxis = (
    'fmax' => 'Fmax (MHz)',
    'fmax2x' => 'Fmax (MHz)',
    'fmaxreal' => 'Fmax (MHz)',
    'fmaxreal2' => 'Fmax (MHz)',
    'alut' => 'ALUTs',
    'reg' => 'Registers',
    'alm' => 'ALMs',
    'dsp' => '18-bit DSP Blocks'
);
foreach my $graph (qw/fmax fmax2x fmaxreal fmaxreal2 alut reg alm dsp/) {
    print GNUPLOT <<EOPLOT;
set output "$graph.$ext"
set xlabel "Pipeline Stages (P)"
set ylabel "$yaxis{$graph}"
#set y2tics mirror
set xtics 1
#set title "$graph - Stratix IV"
EOPLOT
    if ($graph eq 'fmax') {
        #print GNUPLOT "set key right\n";
        print GNUPLOT "set key left\n";
    } else {
        print GNUPLOT "set key left\n";
    }

    print GNUPLOT 'plot ';
    my $first = 1;
    foreach my $width (@widths) {
        foreach my $sign (@signs) {
            #my $signlc = lc $sign;
            my $signlc = substr($sign, 0, 1);

            # combine signed/unsigned for readability
            #if ($width eq '32' || $width eq '8') {
            #if ($width eq '32' || $width eq '8' || $width eq '16') {
            #    next if ($signlc eq 'U');
            #    $signlc = 'U/S';
            #}

            if (!$first) {
                print GNUPLOT ", \\\n";
            }
            $first = 0;
            print GNUPLOT qq("$graph\_$sign\_$width.dat" using 1:2 title "$signlc W=$width" with linespoints);
        }
    }
    print GNUPLOT "\n";
}
close(GNUPLOT);

sub min {
    my $a = shift;
    my $b = shift;
    return $a if ($a < $b);
    return $b;
}
