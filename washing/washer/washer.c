
#include <stdlib.h>
#include <stdio.h>

#include "../libwasher/libwasher.h"

int main ()
{
	struct washer_config_list_node *input_list;
	struct washer_config_list_node *performance_list;

	struct washer_config_list_node *pos;


	if ((input_list = read_configuration ("washer-input.conf")) == NULL)
		return 1;

	if ((performance_list = read_configuration ("washer-times.conf")) == NULL)
		return 1;

	for_each_config_entry (pos, input_list) {
		printf ("input: type = \"%s\", time = %d\n",
			pos->entry.type, pos->entry.val);
	}

	return 0;
}

