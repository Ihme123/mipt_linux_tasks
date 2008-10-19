
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define MAX_STRING_LEN 256


void skip_char_type (char **s, int space)
{
	while (**s && ((isspace (**s) != 0) ? 1 : 0) == space)
		(*s) ++;
}

#define skip_space(s) skip_char_type (&(s), 1)
#define skip_not_space(s) skip_char_type (&(s), 0)

int main ()
{
	char str [MAX_STRING_LEN + 1];
	int delay;
	char *cmd;
	pid_t pid;
	char *cmd_args [20];
	int cmd_arg_c;
	size_t len;

	FILE *fconf = fopen ("useless.conf", "r");
	if (fconf == NULL)
	{
		printf ("configuration file not found!\n");
		return 1;
	}

	while (fgets (str, MAX_STRING_LEN, fconf) != NULL) {
		if (sscanf (str, "%d", &delay) != 1) {
			printf ("unexpected end?\n");
			break;
		}

		len = strlen (str);
		if (str [len - 1] == '\n')
			str [len - 1] = 0;

		cmd = str;
		skip_space (cmd); // spaces before <delay>
		skip_not_space (cmd); // <delay>
		skip_space (cmd); // spaces after <delay>

		cmd_arg_c = 0;
		while (*cmd) {
			printf ("[%s]\n", cmd);
			cmd_args [cmd_arg_c] = cmd;
			cmd_arg_c ++;

			skip_not_space (cmd);
			if (*cmd != '\0') {
				*cmd = '\0';
				cmd ++;
			}
			skip_space (cmd);
		}
		cmd_args [cmd_arg_c] = NULL;

		pid = fork ();
		if (pid == 0) {
			sleep (delay);
			execvp (cmd_args [0], cmd_args);
			printf ("execvp failed! (error: %s)\n", strerror (errno));
		} else if (pid < 0) {
			printf ("fork failed!\n");
		} else {
			// ok, parent
		}
	}
	fclose (fconf);
	return 0;
}

