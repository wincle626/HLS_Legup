#!/bin/sh
#
#  Regression test: Automated install of NetBSD/pmax using R3000 CPU.
#  Starta with:
#
#	test/test_netbsd_pmax_install.sh
#

rm -f nbsd_pmax.img
dd if=/dev/zero of=nbsd_pmax.img bs=1024 count=1 seek=1900000
sync
sleep 2

time test/test_netbsd_pmax_install.expect 2> /tmp/gxemul_result

echo
echo
echo
cat /tmp/gxemul_result
