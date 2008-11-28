
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../lib/lib.h"
#include "../libwasher/libwasher.h"

#define APP_NAME "washer"

void wash (const char *type, int entry_time, int index, int eq_count)
{
	int i;

	printf ("washing \"%s\" (%d/%d): ", type, index + 1, eq_count);
	for (i = 0; i < entry_time; i ++) {
		printf (".");
		fflush (stdout);
	}
	
	printf ("\n");
}

int transport_push_send (struct transport_descriptor *tr,
	const char *msg)
{
	size_t len;
	char *buffer;
	int res;

	len = strlen (msg) + 1;
	buffer = malloc ((len + 20) * sizeof (char));
	if (!buffer) {
		err ("can't alloc buffer");
		return -1;
	}

	sprintf (buffer, "SEND %s", msg);
	res = transport_push (tr, buffer);
	free (buffer);

	return res;
}

int transport_push_quit (struct transport_descriptor *tr)
{
	return transport_push (tr, "QUIT");
}

int main ()
{
	struct transport_descriptor transport;

	struct washer_config_list_node *input_list;
	struct washer_config_list_node *performance_list;

	struct washer_config_list_node *pos;
	int cur_entry_time;
	int i;

	if ((input_list = read_configuration ("washer-input.conf")) == NULL)
		return 1;

	if ((performance_list = read_configuration ("washer.conf")) == NULL)
		return 1;

	printf ("my configuration:\n");
	for_each_config_entry (pos, performance_list) {
		printf ("    type = \"%s\", time = %d\n",
			pos->entry.type, pos->entry.val);
	}

	if (transport_init (&transport, TRANSPORT_FIFO, TRANSPORT_OUT) < 0)
		return 1;

	for_each_config_entry (pos, input_list) {
		printf ("input: type = \"%s\", count = %d\n",
			pos->entry.type, pos->entry.val);

		cur_entry_time = find_config_entry (
			performance_list, pos->entry.type)->val;
		for (i = 0; i < pos->entry.val; i ++) {
			wash (pos->entry.type, cur_entry_time,
				i, pos->entry.val);
			if (transport_push_send (&transport, pos->entry.type) < 0)
				return 1;
		}
	}

	if (transport_push_quit (&transport) < 0)
		return 1;

	return 0;
}

