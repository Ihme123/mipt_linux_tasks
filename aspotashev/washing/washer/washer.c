
#include <stdlib.h>
#include <stdio.h>

#include "../libwasher/libwasher.h"

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
			transport_push (&transport, pos->entry.type);
		}
	}

	return 0;
}

