
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define err(format, args...) \
	fprintf (stderr, "backup error: " \
		format "%s%s\n", \
		## args, \
		errno ? "\n\terror type: " : "", \
		errno ? strerror (errno) : "")

#define info(format, args...) \
	fprintf (stdout, "backup info: " \
		format "\n", ## args)

int backup_dir (const char *backup, const char *source)
{
	DIR *source_dir;
	struct dirent *next;

	source_dir = opendir (source);
	if (source_dir == NULL) {
		err ("can't open source directory: %s", source);
		return -1;
	}

	while ((next = readdir (source_dir)) != NULL) {
		switch (next->d_type) {
		case DT_BLK:  info ("%s is a block device", next->d_name);     break;
		case DT_CHR:  info ("%s is a character device", next->d_name); break;
		case DT_FIFO: info ("%s is a fifo", next->d_name);             break;
		case DT_LNK:  err ("FIXME: symbolic link backups are not implemented! %s", next->d_name); break;
		case DT_REG:  err ("FIXME: reg. file %s", next->d_name); break;
		case DT_SOCK: info ("%s is a socket", next->d_name);           break;
		default: err ("unknown file type");
		}
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

