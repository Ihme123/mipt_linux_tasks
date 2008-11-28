
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define MAX_STRING_LEN 256		/**< stantard string length limit */
#define MAX_PROGRAM_ARGS 20		/**< command arguments number limit */

#define err(format, args...) \
	fprintf (stderr, APP_NAME "[%s:%d] error: " \
		format "%s%s\n", \
		__FILE__, __LINE__, \
		## args, \
		errno ? "\n\terror type: " : "", \
		errno ? strerror (errno) : "")

#define info(format, args...) \
	fprintf (stdout, APP_NAME " info: " \
		format "\n", ## args)


void __skip_char_type (char **s, int space);
#define skip_space(s) __skip_char_type (&(s), 1)
#define skip_not_space(s) __skip_char_type (&(s), 0)

void parse_args (char *s, char *cmd_args []);

