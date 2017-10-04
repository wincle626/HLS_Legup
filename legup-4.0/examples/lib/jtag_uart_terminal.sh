#!/bin/bash

################################################################################
#
# This script (hopefully) locates the Altera installation and then starts the
# nios2 jtag uart terminal. The output of the jtag uart is duplicated and
# written to a file specified by the first argument, $1.
#
################################################################################

# path to nios2 terminal

# Directory of nios2-terminal
nios2_term_dir=`readlink -f $(dirname $(which quartus))`
if [ ! -e $nios2_term_dir/nios2-terminal ] ; then
    # Quartus II 14.0 and earlier have nios2-terminal in a different directory
    nios2_term_dir=`readlink -f $(dirname $(which quartus))/../../nios2eds/bin`
fi

# path to libjtag_atlantic.so
lib_path=`readlink -f $(dirname $(which quartus))/../linux`
if [ ! -e $lib_path/libjtag_atlantic.so ] ; then
    # Quartus II 14.0 and later use the 64-bit version of libjtag_atlantic.so
    lib_path=`readlink -f $(dirname $(which quartus))/../linux64`
fi

# if JTAG_CABLE is unset, set it to a default value
if [ -z "$JTAG_CABLE" ]; then
    # default to first cable
    JTAG_CABLE=`jtagconfig | head -n 1 | cut -c 4-`

    echo "Warning! JTAG_CABLE not set. Using: $JTAG_CABLE."
fi


# launch nios2-terminal
LD_LIBRARY_PATH=$lib_path $nios2_term_dir/nios2-terminal --flush --instance 0 \
    --cable="$JTAG_CABLE" | tee $1

