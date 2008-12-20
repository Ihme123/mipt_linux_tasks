
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#include "../lib/lib.h"

#define APP_NAME "runsim"

struct process_info {
	pid_t pid;

	int stdin_fd;
	int stdout_fd;
	int stderr_fd;

	int id;

	struct process_info *next;
};

#define list_for_each(pos, head) \
	for (pos = head; pos; pos = pos->next)

struct process_info *process_list;

static int running_count;
static int N;
static int runsim_quit;
static int last_proc_id;

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

struct process_info *find_by_id (struct process_info *head, int id)
{
	struct process_info *pos;

	list_for_each (pos, head)
		if (pos->id == id)
			break;

	return pos;
}

int run_internal_command (const char *cmd)
{
	struct process_info *pos;
	struct process_info *proc;
	int id;
	int msg_index;
	ssize_t len;
	char *buffer;

	if (!strcasecmp (cmd, "q")) {
		list_for_each (pos, process_list)
			kill (pos->pid, SIGKILL);

		runsim_quit = 1;
	} else if (cmd [0] == 'w') {
		if (sscanf (cmd + 1, "%d", &id) != 1) {
			err ("wrong input format (id required)");
			return -1;
		}

		msg_index = 1;
		while (isdigit (cmd [msg_index]))
			msg_index ++;
		if (cmd [msg_index] != ' ') {
			err ("wrong input format (after id, msg_index = %d)",
				msg_index);
			return -1;
		}

		msg_index ++;
		proc = find_by_id (process_list, id);
		if (!proc) {
			err ("no process with id = %d", id);
			return -1;
		}

		len = strlen (cmd + msg_index);
		buffer = malloc ((len + 2) * sizeof (char));
		strcpy (buffer, cmd + msg_index);

		strcat (buffer, "\n");
		len ++;

		if (write (proc->stdin_fd, buffer, len) != len) {
			err ("write failed");
			return -1;
		}

		free (buffer);
	} else {
		err ("unknown command");
		return -1;
	}

	return 0;
}

/** Allocates an instance of 'struct process_info' and links it to the list
 */
struct process_info *add_process ()
{
	struct process_info *info;

	if ((info = malloc (sizeof (struct process_info))) == NULL) {
		err ("can't allocate struct process_info");
		return NULL;
	}

	info->next = process_list;
	process_list = info;
	return info;
}

void close_pipe (int fd [2])
{
	close (fd [0]);
	close (fd [1]);
}

/** Executes a program if the number of running programs doesn't exceed N
 *
 * @param cmd_line program with arguments
 */
int try_exec (char *cmd_line)
{
	pid_t pid;
	char *cmd_args [MAX_PROGRAM_ARGS];
	int stdin_pipe [2];
	int stdout_pipe [2];
	int stderr_pipe [2];
	struct process_info *info;

	if (running_count >= N) {
		info ("%d programs are already running", N);
		return -1;
	}

	parse_args (cmd_line, cmd_args);

	if (pipe (stdin_pipe) < 0 || pipe (stdout_pipe) < 0 ||
		pipe (stderr_pipe) < 0) {
		err ("can't create pipes");
		return -1;
	}

	pid = fork ();
	if (pid < 0) {
		close (stdin_pipe [0]);
		close (stdin_pipe [1]);
		close (stdout_pipe [0]);
		close (stdout_pipe [1]);
		close (stderr_pipe [0]);
		close (stderr_pipe [1]);

		err ("fork failed");
		return -1;
	} else if (pid == 0) { // child
		if (dup2 (stdin_pipe [0], 0) < 0 ||
			dup2 (stdout_pipe [1], 1) < 0 ||
			dup2 (stderr_pipe [1], 2) < 0) {
			err ("dup2 failed");
			exit (1);
		}

		close_pipe (stdin_pipe);
		close_pipe (stdout_pipe);
		close_pipe (stderr_pipe);

		execvp (cmd_args [0], cmd_args);

		err ("exec failed");
		exit (1);
	} else { // parent
		close (stdin_pipe [0]);
		close (stdout_pipe [1]);
		close (stderr_pipe [1]);

		info = add_process ();
		info->stdin_fd = stdin_pipe [1];
		info->stdout_fd = stdout_pipe [0];
		info->stderr_fd = stderr_pipe [0];
		info->pid = pid;
		info->id = last_proc_id ++;

		running_count ++;

		info ("started program '%s', id = %d", cmd_line, info->id);
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
	last_proc_id = 1;
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

