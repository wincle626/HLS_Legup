#!/usr/bin/python

#
# elf2sdram.py
#
# Bain Syrowik
# March 10, 2015
#
# This script converts a MIPS elf file to a format that can be used by modelsim.
#
# It is a more generalized replacement of legup/tiger/tool_source/elf2sdram.cpp
#


# TODO
#
# Check inputs to make sure they are reasonable
# ie. mem_size is big enough, mem_start is close(ish) to start_addr, etc.
#
# It might be easier read the objcopy as binary data, WIDTH bits at a time,
# change the endianness, and write it, rather than doing it all with hex strings
#
# TODO


import sys
import subprocess
import shlex
from signal import signal, SIGPIPE, SIG_DFL



def dump_header(ELF_FILE):
    """ Dump the elf header.
    This allows us to get two pieces of information:
    1. Start address of .text section (start of code)
    2. End address of the program + data
    """

    # dump the elf header using objdump
    header = subprocess.Popen("objdump -h %s" % ELF_FILE, shell=True, stdout=subprocess.PIPE).stdout

    # skip to the list of sections
    for line in header:
        if "Idx Name" in line:
            break

    end_addr = 0
    text_VMA = 0
    for line in header:
        vals = line.split()
        # get start address of .text section
        if ".text" in line:
            text_VMA = int(vals[3], 16)
        # for each section, get the end address by adding the size and start VMA
        # and update end_addr
        if len(vals) is 7:
            size = int(vals[2], 16)
            vma  = int(vals[3], 16)
            end_addr = max(size + vma, end_addr)

    return (text_VMA, end_addr)


def dump_elf(ELF_FILE):
    """ Get a dump of the elf file.
    First, use objcopy to dump the data in binary format.
    Second, use xxd to convert the binary data to a string of bytes
    """

    # prevent warning
    signal(SIGPIPE,SIG_DFL)

    # call objcopy
    subprocess.call("objcopy %s -O binary %s.bin -I %s" %
        (ELF_FILE, ELF_FILE, "elf32-little"), shell=True)

    # call xxd
    data = subprocess.Popen("xxd -c 1 -ps %s.bin" % ELF_FILE,
        shell=True, stdout=subprocess.PIPE).stdout.read().split()

    return data



# Convert an avalon-style address to the corresponding ddr2-style address
# TODO: this should probably be generalized for memory widths other than 16
def avalon_to_ddr2_addr(avalon_addr):
    """ Convert an Avalon address to a ddr2 address
    given an avalon address addr[29:0], the corresponding ddr2 address will be
    {addr[15:31], addr[29:16], addr[12:4]}
    """
    ddr_addr = 0
    ddr_addr |= (avalon_addr >>  4) & 0x000001FF
    ddr_addr |= (avalon_addr >>  7) & 0x007FFE00
    ddr_addr |= (avalon_addr << 10) & 0x03800000
    return ddr_addr


def write_data(data, width, MEM_START, DATA_END, SDRAM_FILE, PRINT_ADDR):
    """ Write data to the SDRAM file.
    Lower bytes on the right, higher bytes on the left.
    """

    data_length = DATA_END - MEM_START

    # round data length to be a multiple of the memory width
    if data_length & (width - 1):
        data_length = (data_length & ~(width - 1)) + width

    # pad data so it is the correct length
    pad = data_length - len(data)
    data = data + pad * ["00"]

    address = MEM_START

    # write file
    f = open(SDRAM_FILE, 'w')
    for i in range(0, len(data), width):
        # print the "address" at the beginning of each line, eg.
        # @00000000 <data>
        # @00000001 <data>
        if(PRINT_ADDR):
            f.write("@" + format(avalon_to_ddr2_addr(address), '08x') + " ")
        # write 'width' bytes of data in reverse order
        f.write("".join(data[i:i+width][::-1]))
        f.write("\n")
        address += width
    f.close()


def main(argv = None):
    if argv is None:
        argv = sys.argv

    # check argv and print usage
    if len(argv) != 6:
        print "Incorrect usage.  Expecting:"
        print argv[0], "<MEM_START> <MEM_SIZE> <MEM_WIDTH>",
        print "<elf_file_in> <sdram_file_out>"
        print "<MEM_WIDTH> is the width of output data in bytes"
        return 1

    # get arguments
    MEM_START  = int(argv[1], 0) # automatic conversion from hex OR decimal
    MEM_SIZE   = int(argv[2], 0) # automatic conversion from hex OR decimal
    MEM_WIDTH  = int(argv[3], 0) # automatic conversion from hex OR decimal
    ELF_FILE   = argv[4]
    SDRAM_FILE = argv[5]
    # if we are writing to a .hex file (for DDR2), then we want to print an
    # "address" at the beginning of each line
    PRINT_ADDR = (".hex" in SDRAM_FILE)

    # get start address and data end from elf header
    (START_ADDR, DATA_END) = dump_header(ELF_FILE)

    # calcluate estimated output file size in bytes
    size = DATA_END - MEM_START
    filesize = size * 2 + size / MEM_WIDTH # 2 characters per byte, plus newlines

    # Creating ddr2 .hex files is currently only correct with a MEM_WIDTH of 16
    if PRINT_ADDR and MEM_WIDTH != 16:
        print "\033[0;33m"
        print "elf2sdram.py ERROR:"
        print "Generating a ddr2 .hex file with memory width != 16 may not give"
        print "the correct result."
        print "\033[0m"
        return 1

    # check to make sure program start address and start of memory are sane
    if (START_ADDR - MEM_START) > 0x1000000: # error if > 16MB of padding
        # this limits file size to ~40MB
        print "\033[0;31m"
        print "elf2sdram.py ERROR:"
        print "Program start address (0x%s)" % format(START_ADDR, '08x'),
        print "from .elf file is much larger than"
        print "the start of memory   (0x%s)." % format(MEM_START, '08x')
        print "The output file would be about %d bytes." % filesize,
        print "\033[0m"
        return 1
    elif (START_ADDR - MEM_START) > 0x400: # warn if > 1kB of padding
        print "\033[0;33m"
        print "elf2sdram.py WARNING:"
        print "Program start address (0x%s) from" % format(START_ADDR, '08x'),
        print ".elf file is much larger than"
        print "the start of memory   (0x%s)." % format(MEM_START, '08x')
        print "This will result in > 1kB of extra padding."
        print "The output file will be about %d bytes." % filesize,
        print "\033[0m"

    # get all the data we want to write
    data = dump_elf(ELF_FILE)

    # write the data to the SDRAM file
    write_data(data, MEM_WIDTH, MEM_START, DATA_END, SDRAM_FILE, PRINT_ADDR)



if __name__ == "__main__":
    sys.exit(main())


