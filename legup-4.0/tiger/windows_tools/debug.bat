path=tiger_tools
MIPSLoad %1 localhost
gdb %1 --eval-command="target remote localhost:1337"