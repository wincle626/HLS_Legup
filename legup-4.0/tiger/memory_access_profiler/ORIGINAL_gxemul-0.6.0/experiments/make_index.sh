#!/bin/sh
#
#  $Id: make_index.sh,v 1.1 2007-06-19 02:11:45 debug Exp $
#
#  Updates the .index file.
#

rm -f .index
for a in *.cc; do
	B=`grep COMMENT $a`
	if [ z"$B" != z ]; then
		printf "$a " >> .index
		echo "$B"|cut -d : -f 2- >> .index
	fi
done

