
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#include "../lib/lib.h"

const size_t COPY_BUFFER_SZ = 512;

#define APP_NAME "backup"


int backup_dir_contents (const char *source, const char *backup);

void concat_path (char *output, const char *path_a, const char *path_b)
{
	strcpy (output, path_a);
	strcat (output, "/");
	strcat (output, path_b);
}

int verbose_close (int fd, const char *name)
{
	int result;

	result = close (fd);
	if (result != 0)
		err ("can't close file (%s)", name);

	return result;
}

int run_gzip (const char *pathname)
{
	pid_t pid;
	int status;
	pid_t wait_result;

	pid = fork ();
	if (pid < 0) {
		err ("fork failed");
		return -1;
	} else if (pid == 0) { // child
		execl ("/bin/gzip", "/bin/gzip", pathname, NULL);
		err ("exec failed");
		return -1;
	} else { // parent
		wait_result = wait (&status);
		if (wait_result < 0) {
			err ("wait failed");
			return -1;
		}

		if (status != 0) {
			err ("gzip terminated with error code %d", status);
			return -1;
		}
	}

	return 0;
}

int backup_regular_file (const char *source, const char *backup)
{
	int source_fd; // source file descriptor
	int backup_fd; // backup file descriptor
	u_int8_t buffer [COPY_BUFFER_SZ];
	int result;
	ssize_t read_result;

	source_fd = open (source, O_RDONLY);
	if (source_fd == -1) {
		err ("can't open source file %s", source);
		return -1;
	}

	backup_fd = open (backup,
		O_WRONLY | O_TRUNC | O_CREAT,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (backup_fd == -1) {
		err ("can't create backup file %s", backup);
		verbose_close (source_fd, source);
		return -1;
	}

	result = 0;
	for (;;) {
		read_result = read (source_fd, buffer, COPY_BUFFER_SZ);
		if (read_result < 0) {
			err ("can't read from source file %s", source);
			result = -1;
			break;
		} else if (read_result == 0) { // copy finished
			break;
		}

		if (write (backup_fd, buffer, read_result) != read_result) {
			err ("write to backup file failed\n");
			result = -1;
			break;
		}

		if (read_result < COPY_BUFFER_SZ) { // copy finished
			break;
		}
	}

	if (verbose_close (backup_fd, backup) != 0)
		result = -1;
	if (verbose_close (source_fd, source) != 0)
		result = -1;

	if (result < 0)
		return -1;

	if (run_gzip (backup) < 0)
		return -1;

	return 0;
}

int backup_directory (const char *source, const char *backup)
{
	if (mkdir (backup, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
		if (errno == EEXIST) {
			// this is necessarily a directory,
			// because "backup_object" checks the
			// backup type to match the source file type
			info ("directory %s already exists", backup);
			return 0;
		}

		err ("mkdir failed");
		return -1;
	}

	return backup_dir_contents (source, backup);
}

int backup_object (const char *source, const char *backup)
{
	struct stat source_stat;
	struct stat backup_stat;
	char gzip_backup [MAX_STRING_LEN];
	int result;

	result = lstat (source, &source_stat);
	if (result != 0) {
		err ("source stat failed");
		return -1;
	}

	strcpy (gzip_backup, backup);
	if ((source_stat.st_mode & S_IFMT) == S_IFREG)
		strcat (gzip_backup, ".gz");

	result = lstat (gzip_backup, &backup_stat);
	/* Checking mtime */
	if (result == -1 && errno == ENOENT) {
		info ("creating new backup for %s", source);
	} else if (result == 0) {
		info ("updating backup for %s", source);

		/* if backup already exists and file types don't match */
		if (source_stat.st_mode != backup_stat.st_mode) {
			err ("types of source and backup differ (source: %s, backup: %s)",
				source, gzip_backup);
			return -1;
		}

		if ((source_stat.st_mode & S_IFMT) == S_IFREG &&
			backup_stat.st_mtime >= source_stat.st_mtime) {
			info ("backup file %s is already up-to-date",
				backup_pack);
			return 0;
		}
	} else {
		err ("backup stat failed");
		return -1;
	}

	if ((source_stat.st_mode & S_IFMT) == S_IFREG)
		return backup_regular_file (source, backup);
	else if ((source_stat.st_mode & S_IFMT) == S_IFDIR)
		return backup_directory (source, backup);
	else if ((source_stat.st_mode & S_IFMT) == S_IFLNK)
		info ("symbolic link backups are not implemented (%s)", source);
	else if ((source_stat.st_mode & S_IFMT) == S_IFBLK)
		info ("%s is a block device", source);
	else if ((source_stat.st_mode & S_IFMT) == S_IFCHR)
		info ("%s is a character device", source);
	else if ((source_stat.st_mode & S_IFMT) == S_IFIFO)
		info ("%s is a fifo", source);
	else if ((source_stat.st_mode & S_IFMT) == S_IFSOCK)
		info ("%s is a socket", source);
	else {
		err ("unknown file type (0x%08x): %s",
			source_stat.st_mode, source);
		return -1;
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

