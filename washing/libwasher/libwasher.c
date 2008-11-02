
#include <stdlib.h>
#include <stdio.h>

ssize_t file_read_line (char **line, FILE *f)
{
	size_t len;
	ssize_t res;

	res = getline (line, &len, f);
	if (res == -1) {
		err ();
	}

	return res;
}

struct washer_config_entry *read_configuration (const char *conf_file)
{
	FILE *f = fopen (conf_file, "r");
	if (f == NULL) {
		return NULL;
	}

	fclose (f);
}
