#!/bin/bash

DIR=`pwd`;

HEADER_FILES=`find $DIR -name '*.h'`;

#echo "Header files: $HEADER_FILES"
echo "Mismatches:"
for HEADER_FILE in $HEADER_FILES; do
	SHORTNAME=`basename $HEADER_FILE`
	MATCHNAME=`echo $SHORTNAME|sed 's/\./\\\\./'`
	echo "Investigating references to $SHORTNAME (found at $HEADER_FILE)..."
	grep -rniI [/\"]$MATCHNAME --exclude '*.d' $DIR/* | grep -v $MATCHNAME
done;

echo "Done!";

