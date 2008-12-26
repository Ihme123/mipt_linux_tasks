
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

#define list_for_each(pos, head) \
	for (pos = head; pos; pos = pos->next)

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

int backup_symlink (const char *source, const char *backup)
{
	char symlink_value [MAX_STRING_LEN];
	ssize_t write_res;
	int len;
	int fd;
	int res;

	res = 0;

	if ((len = readlink (source, symlink_value, MAX_STRING_LEN)) < 0) {
		err ("readlink failed");
		return -1;
	}

	if ((fd = open (backup, O_WRONLY | O_TRUNC | O_CREAT,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		err ("can't create backup file for symlink");
		return -1;
	}
	write_res = write (fd, symlink_value, len);
	if (write_res < 0) {
		err ("write failed");
		res = -1;
	} else if (write_res != len) {
		err ("can't write the whole symlink value");
		res = -1;
	}

	if (close (fd) < 0) {
		err ("close failed");
		return -1;
	}

	return res;
}

int backup_hardlink (const char *source, const char *backup)
{
	int fd;
	ssize_t len;

	len = strlen (source);
	if ((fd = open (backup, O_WRONLY | O_TRUNC | O_CREAT,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		err ("can't create backup file for hardlink");
		return -1;
	}
	if (write (fd, source, len) != len) {
		err ("write failed");
		close (fd);
		return -1;
	}

	close (fd);
	return 0;
}

struct file_info {
	dev_t st_dev;
	ino_t st_ino;
	char *filename;

	struct file_info *next;
};

struct file_info *file_list;

struct file_info *find_by_inode (struct file_info *head,
	dev_t st_dev, ino_t st_ino)
{
	struct file_info *pos;

	list_for_each (pos, head)
		if (pos->st_dev == st_dev && pos->st_ino == st_ino)
			break;

	return pos;
}

struct file_info *list_add ()
{
	struct file_info *info;

	if ((info = malloc (sizeof (struct file_info))) == NULL) {
		err ("malloc failed");
		return NULL;
	}

	info->next = file_list;
	file_list = info;
	return info;
}

int backup_object (const char *source, const char *backup)
{
	struct stat source_stat;
	struct stat backup_stat;
	char backup_pack [MAX_STRING_LEN];
	int result;
	struct file_info *orig_file;

	result = lstat (source, &source_stat);
	if (result != 0) {
		err ("source stat failed");
		return -1;
	}

	strcpy (backup_pack, backup);

	if ((orig_file = find_by_inode (file_list,
		source_stat.st_dev, source_stat.st_ino)) != NULL) {
		strcat (backup_pack, ".hardlink");
		backup_hardlink (source, backup_pack);
		info ("hardlink backup: %s -> %s", source, orig_file->filename);
		return 0;
	}

	if ((orig_file = list_add (file_list)) == NULL) {
		err ("list_add failed");
		return -1;
	}
	if ((orig_file->filename = malloc (strlen (source) + 1)) == NULL) {
		err ("malloc failed");
		return -1;
	}
	strcpy (orig_file->filename, source);
	orig_file->st_dev = source_stat.st_dev;
	orig_file->st_ino = source_stat.st_ino;


	if ((source_stat.st_mode & S_IFMT) == S_IFREG)
		strcat (backup_pack, ".gz");

	/* Checking mtime */
	result = lstat (backup_pack, &backup_stat);
	if (result == -1 && errno == ENOENT) {
		info ("creating new backup for %s", source);
	} else if (result == 0) {
		info ("updating backup for %s", source);

		/* if backup already exists and file types don't match */
		if (source_stat.st_mode != backup_stat.st_mode) {
			err ("types of source and backup differ (source: %s, backup: %s)",
				source, backup_pack);
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
	else if ((source_stat.st_mode & S_IFMT) == S_IFLNK) {
		strcat (backup_pack, ".symlink");
		return backup_symlink (source, backup_pack);
	}
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

	file_list = NULL;
	return backup_dir_contents (argv [1], argv [2]);
}

