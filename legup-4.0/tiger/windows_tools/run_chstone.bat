path=tiger_tools
MIPSLoad adpcm.elf localhost -r
MIPSLoad aes.elf localhost -r
MIPSLoad bf.elf localhost -r
MIPSLoad dfadd.elf localhost -r
MIPSLoad dfdiv.elf localhost -r
MIPSLoad dfmul.elf localhost -r
MIPSLoad dfsin.elf localhost -r
MIPSLoad gsm.elf localhost -r
MIPSLoad mips.elf localhost -r
MIPSLoad mpeg2.elf localhost -r
MIPSLoad sha_driver.elf localhost -r

rem  Run last b/c long exec time overlaps next benchmark's programming time
MIPSLoad jpeg.elf localhost -r