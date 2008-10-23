
#include <stdlib.h>
#include <stdio.h>

const size_t MAX_STRING_LEN = 256;

#define err(format, args...) \
	fprintf (stderr, APP_NAME " error: " \
		format "%s%s\n", \
		## args, \
		errno ? "\n\terror type: " : "", \
		errno ? strerror (errno) : "")

#define info(format, args...) \
	fprintf (stdout, APP_NAME " info: " \
		format "\n", ## args)

