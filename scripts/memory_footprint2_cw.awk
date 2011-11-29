
# 
# A small script that parses a codewarrior mapfile and produces
# lists of object files and how much .data and .bss they take.
# 
# Usage: gawk -f <this_file> <mapfile>

function collect( section ) {
    getline
	getline 
	getline 
	while( getline > 0 ) {
		if( $0 ~ /^.$/ ) {
			break;
		}
		
		if ( $6 ~ /^\.[a-z]/ ) {
			curr_obj = $7
			section[curr_obj]=strtonum("0x" $2)
		} 
	}
} 

# sorting
# http://stackoverflow.com/questions/5342782/sort-associative-array-with-awk

function sort_and_print( arr ,  indices ,  tmpidx ) {
	
	for (i in arr) {
		tmpidx[sprintf("%12s", arr[i]),i] = i
	}
	num = asorti(tmpidx)
	j = 0
	for (i=1; i<=num; i++) {
		split(tmpidx[i], tmp, SUBSEP)
		indices[++j] = tmp[2]
	}
	for (i=num; i>0; i--) {
		printf("%35s %d\n",indices[i], arr[indices[i]])
	}
}

/^\.data/{
	collect(data_sec)
}

/^\.bss/{
	collect(bss_sec)
}

END {
	printf " .data\n----------------------------------------------\n"
	sort_and_print(data_sec)

	printf " .bss\n----------------------------------------------\n"
	sort_and_print(bss_sec)
}


