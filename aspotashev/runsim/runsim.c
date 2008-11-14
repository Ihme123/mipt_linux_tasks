
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "../lib/lib.h"

#define APP_NAME "runsim"

static int running_count;
static int N;

void sigchild_handler (int signum)
{
	int status;

	// no synchronization needed,
	// there's no concurrent threads (am I wrong?)
	running_count --;
	wait (&status); // killing zombies!!!
}

int try_run (char *cmd)
{
	pid_t pid;
	char *cmd_args [MAX_PROGRAM_ARGS];

	if (running_count >= N) {
		info ("%d programs are already running", N);
		return -1;
	}

	parse_args (cmd, cmd_args);

	pid = fork ();
	if (pid < 0) {
		err ();
		return -1;
	} else if (pid == 0) { // child
		execvp (cmd_args [0], cmd_args);
		err ("exec failed");
		exit (1);
	} else { // parent
		running_count ++;
		return 0;
	}
}

void usage ()
{
	err ("Usage: runsim <N>");
}

int main (int argc, char *argv [])
{
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

	if (signal (SIGCHLD, sigchild_handler) == SIG_ERR) {
		err ("signal failed");
		return 1;
	}

	running_count = 0;
	while (1) {
		if (fgets (cmd, MAX_STRING_LEN, stdin) == NULL) {
			break;
		}

		len = strlen (cmd);
		if (cmd [len - 1] != '\n') {
			err ("read 'half a string'...exit...");
			break;
		} else {
			cmd [len - 1] = 0;
			try_run (cmd);
		}
	}

	return 0;
}

