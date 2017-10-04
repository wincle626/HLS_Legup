NAME=ignore-mem-deps
clang -emit-llvm -c ${NAME}.c -o ${NAME}.1.bc
../../../llvm/Release+Asserts/bin/llvm-dis ${NAME}.1.bc
../../../llvm/Release+Asserts/bin/opt -basicaa -mem2reg -simplifycfg -loop-simplify -loop-rotate -simplifycfg -instcombine -indvars -da -analyze ${NAME}.1.bc
