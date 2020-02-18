#!/bin/bash
# $1 - HTML source, e.g. html/content.html
#
cdir=`dirname $0`
echo "Converting"
#/c/devtools/Pandoc/pandoc --columns 1000 -r docx -t rst-simple_tables $1 > index.pandoc
mv $1/page* $1/content.html
/c/devtools/Pandoc/pandoc --columns 1000 -r html -t rst $1/content.html > index_pandoc.rst
#cp $1/*.png .

apa=/c/Users/mahi/git/arc-a4/core/scripts/sphinx/
python  $apa/pandoc_to_rst.py index_pandoc.rst > index.rst

#cp index.rst $cdir/../../safety_security/Sm/doc/
#cp *.pnh $cdir/../../safety_security/Sm/doc/










