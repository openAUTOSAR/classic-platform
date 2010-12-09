#
# Get include dirs from:
# touch apa.c 
# gcc -v -c apa.c
#
# "#include <...> search starts here:" to "End of search list."
#

/#include <...> search starts here:/,/End of search list/ {

	if (!(($1=="#include")||($1=="End")))
	  print $0
}



