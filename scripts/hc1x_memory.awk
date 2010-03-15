
# Awk program for parsing "objdump -h" output
# Copyright ArcCore AB

BEGIN {

	# Constants
	kilobyte = 1024

	# Arch constants
	flash_total_size = 128 * kilobytes
	ram_total_size = 8 * kilobytes

	# Setup
	flash_start_hex = "0x4000"
	ram_start_hex = "0x2000"
	flash_start_n = strtonum(flash_start_hex)

	print ""
	print "-- Setup -------------------"
	printf "%-15s %10s\n", "Ram start:", ram_start_hex
	printf "%-15s %10s\n", "Flash start:", flash_start_hex
	print ""
	print "-- Layout ------------------"

	flash_size = 0
	ram_size = 0
}

{
	if ( $1 ~ /[0-9]+/ && $2 ~ /\./ ) {

		name = $2

		size = "0x" $3
		size_n = strtonum(size)

		vma = "0x" $4
		vma_n = strtonum(vma)

		lma = "0x" $5
		lma_n = strtonum(lma)

		type = "-"

		if (lma_n != 0) {
			if (vma_n >= flash_start_n) {
				flash_size += size_n
				type = "flash"
			} else {
				ram_size += size_n
				type = "ram"
			}

			printf "%-10s %10s %10.1f %s %10s\n", name, size_n " B", size_n/kilobyte, "kB", "[" type "]"

		}

	}
}

END {
	print ""
	print "-- Totals ------------------"

	printf "%-10s %10s %10.1f %s\n", "Flash:", flash_size " B", flash_size/kilobyte, "kB"

	printf "%-10s %10s %10.1f %s\n", "Ram:", ram_size " B", ram_size/kilobyte, "kB"
}