#!/bin/bash
# $1 - HTML source, e.g. html/content.html
#
cdir=`dirname $0`
echo "Converting"
#/c/devtools/Pandoc/pandoc --columns 1000 -r docx -t rst-simple_tables $1 > index.pandoc
/c/devtools/Pandoc/pandoc --columns 1000 -r html -t rst-simple_tables $1/content.html > index.pandoc
cp $1/*.png .
python  $cdir/pandoc_to_rst.py index.pandoc > index.rst
cp index.rst $cdir/../../safety_security/Sm/doc/
#cp *.pnh $cdir/../../safety_security/Sm/doc/










