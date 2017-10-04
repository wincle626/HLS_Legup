@ECHO off

if "%1" == "all" (
	call:as_ld adpcm.s
	call:as_ld aes.s
	call:as_ld bf.s
	call:as_ld dfadd.s
	call:as_ld dfdiv.s
	call:as_ld dfmul.s
	call:as_ld dfsin.s
	call:as_ld gsm.s
	call:as_ld jpeg.s
	call:as_ld mips.s
	call:as_ld mpeg2.s
	call:as_ld sha_driver.s
) else if "%1" NEQ "" (
	call:as_ld %1
) else (
	echo "Parameters expected! Either 'all' or benchmark name."
)

goto:eof

:as_ld 
path=tiger_tools
@as %1 -mips1 -mabi=32 -o %~n1.o -EL
@as %1.s -mips1 -mabi=32 -o %1.o -EL

if "%2" == "" (
	rem LINK FOR DE2
	@ld -T lib/prog_link.ld -e main %1.o -o %1.elf -EL -L lib -lgcc -lfloat -luart
) else if "%2" == "-sim" (
	rem LINK FOR SIMULATOR (EL)
	@ld -Ttext 0x80030000 -e main %1.o -o %1.elf -EL -L lib -lgcc -lfloat -luart_el_sim
) else if "%2" == "-sim_eb" (
	rem LINK FOR SIMULATOR (EB)
	@ld -Ttext 0x80030000 -e main %1.o -o %1.elf -L lib -lgcceb -lfloateb -luarteb -lgcc_eb
) 


