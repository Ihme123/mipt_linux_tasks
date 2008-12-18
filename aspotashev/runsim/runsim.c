
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

#include "../lib/lib.h"

#define APP_NAME "runsim"

struct process_info {
	pid_t pid;

	struct process_info *next;
};

#define list_for_each(pos, head) \
	for (pos = head; pos; pos = pos->next)

struct process_info *process_list;

static int running_count;
static int N;
static int runsim_quit;

/** SIGCHLD signal handler
 */
void sigchild_handler (int signum)
{
	int status;

	// no synchronization needed,
	// there's no concurrent threads (am I wrong?)
	running_count --;
	wait (&status); // killing zombies!!!
}

int run_internal_command (const char *cmd)
{
	struct process_info *pos;

	if (!strcasecmp (cmd, "q")) {
		list_for_each (pos, process_list)
			kill (pos->pid, SIGKILL);

		runsim_quit = 1;
		return 0;
	} else {
		err ("unknown command");
		return -1;
	}
}

/** Executes a program if the number of running programs doesn't exceed N
 *
 * @param cmd_line program with arguments
 */
int try_exec (char *cmd_line)
{
	pid_t pid;
	char *cmd_args [MAX_PROGRAM_ARGS];

	if (running_count >= N) {
		info ("%d programs are already running", N);
		return -1;
	}

	parse_args (cmd_line, cmd_args);

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

int try_run (char *cmd)
{
	if (cmd && cmd [0] == '/' && cmd [0] != '\0' && cmd [1] == ' ')
		return run_internal_command (cmd + 2);
	else return try_exec (cmd);
}

/** Prints help on program usage
 */
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
	process_list = NULL;
	runsim_quit = 0;
	while (!runsim_quit) {
		if (fgets (cmd, MAX_STRING_LEN, stdin) == NULL) {
			break;
		}

		len = strlen (cmd);
		if (cmd [len - 1] != '\n') {
			err ("read \'half a string\' (string is "
				"probably too long)...exit...");
			break;
		} else {
			cmd [len - 1] = 0;
			try_run (cmd);
		}
	}

	return 0;
}

