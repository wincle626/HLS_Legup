#!/bin/bash
#This script generates all systems (single, pthreads, pthreads+openmp, etc) for all of the benchmark
#It basically calls generateSystem.sh for each benchmark. Scripts are launched in parallel. 

# EXECUTION COMMAND
parallel -j 4 <<EOF
  ./generateSystems.sh mandelbrot; echo "Mandelbrot finished"
  ./generateSystems.sh divstore; echo "Divstore finished"
  ./generateSystems.sh primestore; echo "Primestore finished"
  ./generateSystems.sh hash; echo "Hash finished"
  ./generateSystems.sh los2; echo "Los2 finished"
  ./generateSystems.sh dfsin; echo "Df finished"
  ./generateSystems.sh blackscholes; echo "Blackscholes finished"
EOF
