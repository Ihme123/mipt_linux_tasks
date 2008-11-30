
#define _GNU_SOURCE
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "../../lib/lib.h"
#include "libwasher.h"

#define APP_NAME "libwasher/config"


static ssize_t file_read_line (char **line, FILE *f)
{
	size_t len;
	ssize_t res;

	*line = NULL;
	res = getline (line, &len, f); // FIXME: getline is GNU only
	if (res == -1) {
//		err ("can't read line");
	}

	return res;
}

static int config_line_ok (const char *line)
{
	return strchr (line, ':') != NULL;
}

// allocate and attach to the list
static struct washer_config_list_node *washer_config_list_alloc (
	struct washer_config_list_node *list)
{
	struct washer_config_list_node *node;

	node = malloc (sizeof (struct washer_config_list_node));
	node->next = list;

	return node;
}

struct washer_config_list_node *read_configuration (const char *conf_file)
{
	FILE *f;
	int N;
	char *line;
	char *value_str;

	struct washer_config_list_node *node;
	struct washer_config_entry *entry;

	f = fopen (conf_file, "r");
	if (f == NULL) {
		err ("read_configuration: can't open file %s", conf_file);
		return NULL;
	}

	N = 0;
	node = NULL;
	while (1) {
		if (file_read_line (&line, f) < 0 || line == NULL)
			break;

		if (config_line_ok (line)) {
			node = washer_config_list_alloc (node);
			entry = &node->entry;

			value_str = strchr (line, ':');
			*value_str = '\0'; // finish type name string
			entry->type = malloc (strlen (line) + 1);
			strcpy (entry->type, line);

			value_str ++;
			sscanf (value_str, "%d", &entry->val);
		}
		else info ("configuration file syntax error: %s", line);

		free (line);
	}

	fclose (f);
	return node;
}


int get_table_limit ()
{
	char *env_ptr;
	char env [20];
	char test [20];
	int res;

	env_ptr = getenv ("TABLE_LIMIT");
	if (env_ptr == NULL) {
		err ("TABLE_LIMIT not set");
		return -1;
	}

	strncpy (env, env_ptr, 19);
	res = atoi (env);
	sprintf (test, "%d", res);
	if (strcmp (test, env)) {
		err ("get_table_limit: bad TABLE_LIMIT format,"
			"should be a decimal integer number");
		return -1;
	} else if (res < 1 || res > 1000) {
		err ("get_table_limit: table is too small or too big");
		return -1;
	} else { // ok
		return res;
	}
}

