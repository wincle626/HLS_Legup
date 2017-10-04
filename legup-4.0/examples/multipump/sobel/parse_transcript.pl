#!/usr/bin/perl

open(TRANSCRIPT, '<transcript') || die "Error: $!\n";

open(PGM, '>hw.pgm') || die "Error: $!\n";

my $start = 0;
while (my $line = <TRANSCRIPT>) {
    if (!$start && $line =~ /P2/) {
        $start = 1;
    } 

    next unless ($start);

    last if ($line =~ /At t=/);

    $line =~ s/^#\s+//;
    $line =~ s/\s+/ /g;
    print PGM "$line\n";

}
close(TRANSCRIPT);
close(PGM);
