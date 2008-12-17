
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

#define APP_NAME "drier"

#include "../../lib/lib.h"
#include "../libwasher/libwasher.h"

#define CALL_CHECKED_RET(p,ret) {	\
	if (p) {			\
		err ("fail: #p");	\
		return ret;		\
	}				\
}

#define CALL_CHECKED(p) CALL_CHECKED_RET(p, -1)
#define CALL_CHECKED_P(p) CALL_CHECKED_RET(p, NULL)

///* this probably should be a towel,
// * but what's the software with no kitchen sinks? */
//pthread_mutex_t kitchen_sink;

int table_limit;

char_msg_t *msg_stack;
int msg_stack_N;

sem_t empty_sem;
sem_t full_sem;
pthread_mutex_t msg_stack_lock;

void dry (const char *type, int entry_time)
{
	int i;

	printf ("drying \"%s\" (it takes %d ticks): ", type, entry_time);
	for (i = 0; i < entry_time; i ++) {
		usleep (300000);

		printf (".");
		fflush (stdout);
	}
	
	printf ("\n");
}

void *communication_thread (void *ptr)
{
	struct transport_descriptor transport;
	char_msg_t *new_msg;

	CALL_CHECKED_P ((new_msg = malloc (sizeof (char_msg_t))) == NULL);

	if (transport_init (&transport, get_tr_type (), TRANSPORT_IN) < 0) {
		free (new_msg);
		return 0;
	}

	while (1) {
		if (transport_pull (&transport, *new_msg) < 0) {
			err ("can't pull");
			strcpy (*new_msg, "QUIT");
		}

		// adding message (pushing to stack)
		CALL_CHECKED_P (sem_wait (&full_sem)); // waiting while the table is full

		CALL_CHECKED_P (pthread_mutex_lock (&msg_stack_lock));
		strcpy (msg_stack [msg_stack_N], *new_msg);
		msg_stack_N ++;
		CALL_CHECKED_P (pthread_mutex_unlock (&msg_stack_lock));

		CALL_CHECKED_P (sem_post (&empty_sem));


		if (!strcmp (*new_msg, "QUIT"))
			break;
	}

	free (new_msg);
	return 0;
}

int buffers_init ()
{
	CALL_CHECKED ((msg_stack = malloc (table_limit * sizeof (char_msg_t))) == NULL);

	return 0;
}

int buffers_destroy ()
{
	free (msg_stack);
	msg_stack = NULL;

	return 0;
}

int sync_init ()
{
	CALL_CHECKED (pthread_mutex_init (&msg_stack_lock, NULL));

	CALL_CHECKED (sem_init (&empty_sem, 1, 0));
	CALL_CHECKED (sem_init (&full_sem, 1, table_limit));

	return 0;
}

int sync_destroy ()
{
	CALL_CHECKED (sem_destroy (&full_sem));
	CALL_CHECKED (sem_destroy (&empty_sem));

	CALL_CHECKED (pthread_mutex_destroy (&msg_stack_lock));

	return 0;
}

int main ()
{
	struct washer_config_list_node *performance_list;
	struct washer_config_list_node *pos;

	int cur_entry_time;

	pthread_t thread;
	int washer_quit;

	char_msg_t *new_msg;

	if ((performance_list = read_configuration ("drier.conf")) == NULL)
		return 1;

	printf ("my configuration:\n");
	for_each_config_entry (pos, performance_list) {
		printf ("    type = \"%s\", time = %d\n",
			pos->entry.type, pos->entry.val);
	}

	if ((table_limit = get_table_limit ()) < 0) {
		err ("you've broken my table! check TABLE_LIMIT");
		return 1;
	}

	CALL_CHECKED (buffers_init ());
	CALL_CHECKED (sync_init ());

	CALL_CHECKED ((new_msg = malloc (sizeof (char_msg_t))) == NULL);

	CALL_CHECKED (pthread_create (&thread, NULL, communication_thread, NULL));

	msg_stack_N = 0;
	washer_quit = 0;
	while (1) {
		if (washer_quit) {
			if (sem_trywait (&empty_sem) != 0) {
				if (errno == EAGAIN) {
					info ("Quitting...");
					break;
				} else {
					err ("sem_trywait failed");
					return 1;
				}
			}
		}
		else { // if semaphore haven't been locked by sem_trywait
			CALL_CHECKED (sem_wait (&empty_sem));
		}

		// get something from the table
		CALL_CHECKED (pthread_mutex_lock (&msg_stack_lock));
		strcpy (*new_msg, msg_stack [msg_stack_N - 1]);
		msg_stack_N --;
		CALL_CHECKED (pthread_mutex_unlock (&msg_stack_lock));

		CALL_CHECKED (sem_post (&full_sem));

		if (!strcmp (*new_msg, "QUIT")) { // quit
			washer_quit = 1;
			break;
		} else if (!strncmp (*new_msg, "SEND ", 5)) { // dry
			cur_entry_time = find_config_entry (
				performance_list, *new_msg + 5)->val;

			dry (*new_msg + 5, cur_entry_time);
		} else {
			err ("bad message");
			return 1;
		}
	}

	CALL_CHECKED (pthread_join (thread, NULL));

	if (sync_destroy () < 0) return 1;
	if (buffers_destroy () < 0) return 1;

	return 0;
}

