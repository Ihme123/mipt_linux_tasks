
#include <stdlib.h>

#include "../lib/lib.h"

#define APP_NAME "runsim"

int try_run (const char *cmd)
{
	info ("cmd = %s", cmd);
	return 0;
}

void usage ()
{
	err ("Usage: runsim <N>");
}

int main (int argc, char *argv [])
{
	int N;
	char reverse_conv [MAX_STRING_LEN];
	char cmd [MAX_STRING_LEN + 1];
	size_t len;

	if (argc != 2) {
		err ("this program takes 1 argument");
		usage ();
		return 1;
	}

	N = atoi (argv [1]);
	sprintf (reverse_conv, "%d", N);
	if (strcmp (argv [1], reverse_conv)) {
		err ("input argument presentation error: (%s), (%s)",
			argv [1], reverse_conv);
		usage ();
		return 1;
	}

	while (1) {
		if (fgets (cmd, MAX_STRING_LEN, stdin) == NULL) {
			err ("exit...");
		}

		len = strlen (cmd);
		if (cmd [len - 1] != '\n') {
			err ("read 'half a string'...exit...");
		} else {
			cmd [len - 1] = 0;
			try_run (cmd);
		}
	}

	return 0;
}

