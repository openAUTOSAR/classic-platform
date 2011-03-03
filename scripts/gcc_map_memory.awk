
BEGIN {
	kilobyte = 1024
	
	print "  >> Image size: (decimal)"
}

/^\.text/ {
	text += $3; 
	rom+=$3 
};

/^\.data/ {
 	data += $3;
	rom+=$3; 
	ram+=$3
};

/^\.bss/ {
	bss += $3; 
	ram+=$3
};

END { 

	printf "%-10s %10s %10.1f %s\n", "  text:", text " B", text/kilobyte, "kB";
	printf "%-10s %10s %10.1f %s\n", "  data:", data " B", data/kilobyte, "kB";
	printf "%-10s %10s %10.1f %s\n", "  bss:", bss " B", bss/kilobyte, "kB";	
	printf "%-10s %10s %10.1f %s\n", "  ROM:", rom " B", rom/kilobyte, "kB";
	printf "%-10s %10s %10.1f %s\n", "  RAM:", ram " B", ram/kilobyte, "kB";
	
}