#!/bin/bash

# Initial Setup for Scinet and launch script for each benchmark 
#   ./launch.sh <output_directory>

if [ -z "$1" ]
then
echo "./launch.sh <output_directory>"
exit
fi

# benchmarks with DSPs
#BENCHMARKS=( chstone/adpcm )
#BENCHMARKS=( chstone/adpcm chstone/dfdiv chstone/dfmul chstone/dfsin chstone/gsm chstone/jpeg chstone/mips dhrystone)
#BENCHMARKS=( chstone/gsm chstone/jpeg )
#BENCHMARKS=( chstone/dfmul )

#BENCHMARKS=( multipump/alphablend multipump/gaussblur multipump/idct \
#             multipump/mandelbrot multipump/matrixmultiply multipump/sobel )
#BENCHMARKS=( multipump/mandelbrot )

# Which benchmark from the legup/examples do you want to run?
# Each benchmark is passed to job.sh
BENCHMARKS=( \
multipump/alphablend multipump/alphablend_rs multipump/alphablend_mp \
multipump/gaussblur multipump/gaussblur_rs multipump/gaussblur_mp \
multipump/idct multipump/idct_rs multipump/idct_mp \
multipump/mandelbrot multipump/mandelbrot_rs multipump/mandelbrot_mp \
multipump/matrixmultiply multipump/matrixmultiply_rs multipump/matrixmultiply_mp \
multipump/sobel multipump/sobel_rs multipump/sobel_mp \
)

DIR=$PWD/$1
HOME=$PWD
if [ -d "$DIR" ]
then
    echo "Re-running $DIR"
    cd $DIR
    mkdir -p old
    mv examples old
    mv legup-multipump.* old
    cd $HOME
else
    echo "Setting up $DIR"
    mkdir -p $DIR
    cp job.sh $DIR

    # get a snapshot of the legup
    LEGUP=$DIR/legup
    mkdir -p $LEGUP
    # copy legup binaries and benchmark to Ramdisk
    #cd /home/j/janders/jchoi/legup-2.0/
    #cd /scratch/j/janders/jchoi/andrew/legup/
    cd /scratch/j/janders/acanis/legup/
    make -j18
    cp -v -r --parents llvm/Release+Asserts $LEGUP
    cp -v --parents examples/* $LEGUP
    for (( a = 0 ; a < ${#BENCHMARKS[@]} ; a++ ))
    do
        cp -v -r --parents examples/${BENCHMARKS[$a]} $LEGUP
    done
    cp -v -r --parents examples/lib $LEGUP
    cp -v -r --parents hwtest $LEGUP
    cp -v -r --parents cloog/install $LEGUP
    cp -v -r --parents llvm/tools/polly/Release+Asserts $LEGUP
    cp -v -r --parents tiger/processor/tiger_hybrid_pipelined_new/../altera_libs/ $LEGUP
    cp -v -r --parents llvm/lib/Target/Verilog $LEGUP
    cp -v -r --parents llvm/lib/Transforms/LegUp $LEGUP
    git diff > $LEGUP/git_diff
fi

cd $DIR
for (( a = 0 ; a < ${#BENCHMARKS[@]} ; a++ ))
do
		echo ${BENCHMARKS[$a]}
		qsub -v BENCHMARK="${BENCHMARKS[$a]}" job.sh &
		sleep 3
done
