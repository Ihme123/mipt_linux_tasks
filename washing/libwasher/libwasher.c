
#include <stdlib.h>
#include <stdio.h>

#include "../../lib/lib.h"

#define APP_NAME "libwasher"


ssize_t file_read_line (char **line, FILE *f)
{
	size_t len;
	ssize_t res;

	res = getline (line, &len, f);
	if (res == -1) {
		err ("can't read line");
	}

	return res;
}

int config_line_ok (const char *line)
{
	return strchr (line, ':') != NULL;
}

void for_each_line (FILE *f, void (*func)(const char *, void *), void *arg)
{
	char *line;

	rewind (f);
	while (!feof (f)) {
		file_read_line (&line, f);
		if (config_line_ok (line))
			func (line, arg);
	}
}

void count_line (const char *line, void *N)
{
	(*((int*)N)) ++;
}

struct washer_config_entry *read_configuration (const char *conf_file)
{
	FILE *f;
	int N;
	char *line;
	char *value_str;

	struct washer_config_entry entry;

	f = fopen (conf_file, "r");
	if (f == NULL) {
		return NULL;
	}

	N = 0;
	while (!feof (f)) {
		file_read_line (&line, f);
		if (config_line_ok (line)) {
			value_str = strchr (line, ':');
			*value_str = '\0'; // finish type name string
			entry.type = malloc (strlen (line) + 1);
			strcpy (entry.type_name, line);

			value_str ++;
			sscanf (value_str, "%d", &entry.val);

			list_add (...);
		}
	}

	fclose (f);
}
