
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define err(format, args...) \
	fprintf (stderr, "backup: " \
		format "%s%s\n", \
		## args, \
		errno ? "\n\terror type: " : "", \
		errno ? strerror (errno) : "")

int backup_dir (const char *backup, const char *source)
{
	DIR *source_dir;
	struct dirent *dp;

	source_dir = opendir (source);
	if (source_dir == NULL) {
		err ("can't open source directory: %s", source);
		return -1;
	}

	while ((dp = readdir (dir)) != NULL) {
		...
	}

	return 0;
}

int main (int argc, char *argv [])
{
	err ("my error - 0x%x", 100);

	if (argc != 3) {
		err ("Usage: backup <src-dir> <dest-dir>");
		return 1;
	}

	backup_dir (argv [2], argv [1]);

	return 0;
}

