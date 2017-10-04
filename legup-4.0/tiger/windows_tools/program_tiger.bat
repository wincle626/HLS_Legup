@path=C:/altera/90sp2/quartus/bin

set PROJECT=C:/altera/tiger/tiger_mips/tiger_top
quartus_pgm -c USB-Blaster -m jtag -o p;%1

@path=C:/altera/90sp2/quartus/bin
@rem quartus_stp -t lib/serial.tcl "USB-Blaster [USB-0]" 0 1 1225