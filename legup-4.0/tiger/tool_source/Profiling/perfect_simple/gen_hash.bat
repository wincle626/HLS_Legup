@ECHO off

if "%1" == "all" (
	call:gen_hash aes
	call:gen_hash adpcm
	call:gen_hash bf
	call:gen_hash dfadd
	call:gen_hash dfdiv
	call:gen_hash dfmul
	call:gen_hash dfsin
	call:gen_hash gsm
	call:gen_hash jpeg
	call:gen_hash mips
	call:gen_hash mpeg2
	call:gen_hash sha_driver
) else if "%1" NEQ "" (
	call:gen_hash %1
) else (
	echo "Parameters expected! Either 'all' or benchmark name."
)

goto:eof

:gen_hash
	rem ./gen_func_addr $1.elf $1.s > $1.flist
	echo %1
	perfect 32 < ../%1.flist
	mv phash.c ../%1.phash.c
	rem mv phash.h ../%1.phash.h
	mv phash.prof ../%1.phash.prof