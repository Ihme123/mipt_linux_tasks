
#include <stdlib.h>
#include <stdio.h>

#include <fcntl.h>
#include <sys/stat.h>

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

static int transport_ok (struct transport_descriptor *tr)
{
	return tr->dir == TRANSPORT_IN || tr->dir == TRANSPORT_OUT;
}

static int transport_init_fifo (struct transport_descriptor *tr)
{
	const char filename [] = "transport-fifo";

	if (tr->dir == TRANSPORT_OUT) {
		if (mkfifo (filename, S_IWUSR | S_IRUSR) < 0) {
			if (errno == EEXIST)
				info ("fifo already exists");
			else {
				err ("can't create fifo");
				return -1;
			}
		}
	}

	if ((tr->fd = open (filename, tr->dir == TRANSPORT_OUT ?
		O_WRONLY : O_RDONLY)) < 0) {
		err ("can't open fifo");
		return -1;
	}

	return 0;
}

int transport_init (struct transport_descriptor *tr,
	enum TRANSPORT_TYPES type, enum TRANSPORT_DIRECTIONS dir)
{
	tr->type = type;
	tr->dir = dir;

	if (!transport_ok (tr))
		return -1;

	switch (type) {
	case TRANSPORT_FIFO: return transport_init_fifo (tr);
	default: err ("bad transport type"); return -1;
	}
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

static int transport_push_fifo (struct transport_descriptor *tr,
	const char *msg)
{
	size_t len;
	ssize_t res;

	len = strlen (msg) + 1;

	res = write (tr->fd, msg, len);
	if (res < 0) {
		err ("can't write to fifo");
		return -1;
	} else if (res != len) {
		err ("writing to fifo: partial success");
		return -1;
	}

	return 0;
}

int transport_push (struct transport_descriptor *tr, const char *msg)
{
	if (!transport_ok (tr))
		return -1;

	switch (tr->type) {
	case TRANSPORT_FIFO: return transport_push_fifo (tr, msg);
	default: err ("bad transport type"); return -1;
	}
	return 0;
}

