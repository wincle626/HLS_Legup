@path=C:/altera/90sp2/quartus/bin

set PROJECT=C:/altera/tiger/tiger_mips/tiger_top
quartus_cdb %PROJECT% -c %PROJECT% --update_mif
quartus_asm --read_settings_files=on --write_settings_files=off %PROJECT% -c %PROJECT%