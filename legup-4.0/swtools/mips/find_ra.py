#!/usr/bin/python

import sys

def main(argv = None):
    if argv is None:
        argv = sys.argv

    # check argv and print usage
    if len(argv) != 2:
        print "Incorrect usage.  Expecting:"
        print argv[0], "<mips_source>"
        return 1

    found_main = False
    found_jr_ra = False
    try:
        f = open(argv[1], 'r')
    except IOError:
        print "File not found! (%s) Quitting." % argv[1]
        return 1
    else:
        with f:
            for l in f:
                if "<main>:" in l:
                    found_main = True
                    break
            for l in f:
                if "jr" in l and "ra" in l:
                    found_jr_ra = True
                    print "0xffffffff%s" % l.split()[0][0:8]
                    break
        f.close()
    if not found_main:
        print "Main function not found!! ERROR!"
        return 1
    if not found_jr_ra:
        print "'jr ra' call not found!! ERROR!"
        return 1


if __name__ == "__main__":
    sys.exit(main())

