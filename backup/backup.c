
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>

#define err(format, args...) \
	fprintf (stderr, format "\n", ## args)

void backup_dir (const char *backup, const char *source)
{
	DIR *source_dir;

	source_dir = opendir (source);
	if (source_dir == NULL) {
		err ("error - 0x%x", 100);
	}
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

