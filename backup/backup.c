
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>

const size_t MAX_STRING_LEN = 256;

#define err(format, args...) \
	fprintf (stderr, "backup error: " \
		format "%s%s\n", \
		## args, \
		errno ? "\n\terror type: " : "", \
		errno ? strerror (errno) : "")

#define info(format, args...) \
	fprintf (stdout, "backup info: " \
		format "\n", ## args)

void concat_path (char *output, const char *path_a, const char *path_b)
{
	strcpy (output, path_a);
	strcat (output, "/");
	strcat (output, path_b);
}

int backup_object (const char *source, const char *backup)
{
	info ("d_name = [%s]", source);

	struct stat file_stat;
	if (stat (next->d_name) != 0) {
		err ("stat failed");
		return -1;
	}

	switch (next->d_type) {
	case DT_BLK:  info ("%s is a block device", next->d_name);     break;
	case DT_CHR:  info ("%s is a character device", next->d_name); break;
	case DT_FIFO: info ("%s is a fifo", next->d_name);             break;
	case DT_LNK:  err ("FIXME: symbolic link backups are not implemented! %s", next->d_name); break;
	case DT_REG:  err ("FIXME: reg. file %s", next->d_name); break;
	case DT_SOCK: info ("%s is a socket", next->d_name);           break;
	default: err ("unknown file type");
	}

	return 0;
}

/** backup all objects in the given directory
 *
 */
int backup_dir_contents (const char *source, const char *backup)
{
	DIR *source_dir;
	struct dirent *next;
	char source_obj_path [MAX_STRING_LEN];
	char backup_obj_path [MAX_STRING_LEN];

	source_dir = opendir (source);
	if (source_dir == NULL) {
		err ("can't open source directory: %s", source);
		return -1;
	}

	while ((next = readdir (source_dir)) != NULL) {
		if (!strcmp (next->d_name, ".") || !strcmp (next->d_name, ".."))
			info ("skipping \"%s\"", next->d_name);
		else {
			concat_path (source_obj_path, source, next->d_name);
			concat_path (backup_obj_path, backup, next->d_name);
			backup_object (source_obj_path, backup_obj_path);
		}
	}

	return 0;
}

int main (int argc, char *argv [])
{
	if (argc != 3) {
		err ("Usage: backup <src-dir> <dest-dir>");
		return 1;
	}

	backup_dir_contents (argv [1], argv [2]);

	return 0;
}

