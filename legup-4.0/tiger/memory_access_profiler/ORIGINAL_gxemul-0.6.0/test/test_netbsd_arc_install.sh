#!/bin/sh
#
#  Regression test: Automated install of NetBSD/arc 5.0. Start using:
#
#	test/test_netbsd_arc_install.sh
#

rm -f nbsd_arc.img
dd if=/dev/zero of=nbsd_arc.img bs=1024 count=1 seek=1000000

time test/test_netbsd_arc_install.expect 2> /tmp/gxemul_result

echo
echo
cat /tmp/gxemul_result

