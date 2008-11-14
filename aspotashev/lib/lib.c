
#include <ctype.h>

#include "lib.h"

void __skip_char_type (char **s, int space)
{
	while (**s && ((isspace (**s) != 0) ? 1 : 0) == space)
		(*s) ++;
}

void parse_args (char *s, char *cmd_args [])
{
	int cmd_arg_c;

	skip_space (s);

	cmd_arg_c = 0;
	while (*s) {
//		printf ("[%s]\n", s);
		cmd_args [cmd_arg_c] = s;
		cmd_arg_c ++;

		skip_not_space (s);
		if (*s != '\0') {
			*s = '\0';
			s ++;
		}
		skip_space (s);
	}
	cmd_args [cmd_arg_c] = NULL;
}

