
#include <stdlib.h>
#include <stdio.h>

#include "../../lib/lib.h"
#include "libwasher.h"

#define APP_NAME "libwasher"


static ssize_t file_read_line (char **line, FILE *f)
{
	size_t len;
	ssize_t res;

	*line = NULL;
	res = getline (line, &len, f);
	if (res == -1) {
//		err ("can't read line");
	}

	return res;
}

static int config_line_ok (const char *line)
{
	return strchr (line, ':') != NULL;
}

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
			node = washer_config_list_alloc (node); // allocate and attach to the list
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

int transport_init (struct transport_descriptor *transport,
	enum TRANSPORT_TYPES type, enum TRANSPORT_DIRECTIONS dir)
{
	if (dir != TRANSPORT_IN && dir != TRANSPORT_OUT)
		return -1;

	switch (type) {
	case TRANSPORT_FIFO:
	default: err ("bad transport type"); return -1;
	}

	transport->type = type;
	transport->dir = type;
	return 0;
}

struct washer_config_entry *find_config_entry (
	struct washer_config_list_node *list, const char *type)
{
	struct washer_config_list_node *pos;

	for_each_config_entry (pos, list)
		if (!strcmp (pos->entry.type, type))
			return &pos->entry;
	
	err ("entry not found");
	return NULL;
}

int transport_ok (struct transport_descriptor *tr)
{
	return tr->dir == TRANSPORT_IN || tr->dir == TRANSPORT_OUT;
}

int transport_push (struct transport_descriptor *tr, const char *type)
{
	if (!transport_ok (tr))
		return -1;

	switch (tr->type) {
	case TRANSPORT_FIFO:
	default: err ("bad transport type"); return -1;
	}

	return 0;
}

