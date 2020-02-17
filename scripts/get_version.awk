/^#define[ \t]+_ARCTIC_CORE_MAJOR_/ {
	core_major=strtonum($3)
}
/^#define[ \t]+_ARCTIC_CORE_MINOR_/ {
	core_minor=strtonum($3)
}

/^#define[ \t]+_ARCTIC_CORE_PATCHLEVEL_/ {
	core_patch=strtonum($3)
}

/^#define[ \t]+_ARCTIC_CORE_BUILDTYPE_/ {
	core_buildtype=$3
}

END {
	printf "v%d%c%d%c%d%c%s\n", core_major,type,core_minor,type,core_patch,type,core_buildtype;
}

