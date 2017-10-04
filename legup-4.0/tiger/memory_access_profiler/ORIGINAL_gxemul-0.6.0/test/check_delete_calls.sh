#!/bin/sh

rm -f tmp_delete.out
LC_ALL=C grep -R "delete " src|grep -v \\.svn|cut -d \: -f 1|sort|uniq |grep -v "inary file" > tmp_delete.out 2> /dev/null

ANYERRORS=0

for a in `cat tmp_delete.out`; do
	# Is $a a known exception?
	if grep -q $a test/check_delete_calls.exceptions; then
		:
	else
		printf "\nError: $a is not in check_delete_calls.exceptions!"
		ANYERRORS=1
	fi
done

rm -f tmp_delete.out

if [ z$ANYERRORS = z1 ]; then
	printf "\n\n"
	false
fi

