
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

int backup_regular_file (const char *source, const char *backup)
{
	int fd;

	fd = open (source, O_RDONLY);
	close (fd);
}

int backup_object (const char *source, const char *backup)
{
	struct stat file_stat;

	info ("d_name = [%s]", source);
	
	if (stat (source, &file_stat) != 0) {
		err ("stat failed");
		return -1;
	}

	switch (file_stat.st_mode) {
	case S_IFREG:  return backup_regular_file (source, backup);
	case S_IFLNK:  info ("symbolic link backups are not implemented (%s)", source); break;

	case S_IFBLK:  info ("%s is a block device", source);     break;
	case S_IFCHR:  info ("%s is a character device", source); break;
	case S_IFIFO:  info ("%s is a fifo", source);             break;
	case S_IFSOCK: info ("%s is a socket", source);           break;

	default: err ("unknown file type: %s", source);           break;
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
			if (backup_object (source_obj_path, backup_obj_path) < 0)
				return -1;
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

	return backup_dir_contents (argv [1], argv [2]);
}

