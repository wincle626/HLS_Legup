#!/bin/sh
#
#  Regression test  --  automated install of OpenBSD/landisk
#  Start with:
#
#	test/test_openbsd_landisk_install.sh
#

rm -f obsd_landisk.img
dd if=/dev/zero of=obsd_landisk.img bs=1024 count=1 seek=1900000
sync
sleep 2

time test/test_openbsd_landisk_install.expect 2> /tmp/gxemul_result

echo
echo
echo
echo
cat /tmp/gxemul_result

