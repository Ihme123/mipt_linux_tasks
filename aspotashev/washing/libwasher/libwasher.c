
#include <ctype.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../../lib/lib.h"
#include "libwasher.h"
#include "fifo.h"

#define APP_NAME "libwasher"


static int transport_ok (struct transport_descriptor *tr)
{
	return tr->dir == TRANSPORT_IN || tr->dir == TRANSPORT_OUT;
}

int is_sending_transport (enum TRANSPORT_DIRECTIONS dir, int ack)
{
	return (dir == TRANSPORT_OUT) ? (ack == 0) : (ack != 0);
}

static int transport_init_prio_dir (
	int (* one_way_init)(struct one_way_transport *, int, enum TRANSPORT_DIRECTIONS),
	struct transport_descriptor *tr,
	int prio) // current behaviour: reading channel = first
{
	if ((prio == 0 && tr->dir == TRANSPORT_IN) || (prio == 1 && tr->dir == TRANSPORT_OUT)) {
		return one_way_init (&tr->fw, 0, tr->dir);
	} else {
		return one_way_init (&tr->ack, 1, tr->dir);
	}
}

int transport_init (struct transport_descriptor *tr,
	enum TRANSPORT_TYPES type, enum TRANSPORT_DIRECTIONS dir)
{
	int (* one_way_init)(struct one_way_transport *, int, enum TRANSPORT_DIRECTIONS);
	int (* common_init)(struct transport_descriptor *);

	tr->type = type;
	tr->dir = dir;

	if (!transport_ok (tr))
		return -1;

	switch (type) {
	case TRANSPORT_FIFO: one_way_init = transport_init_fifo_dir; common_init = transport_init_fifo_common; break;
	default: err ("bad transport type"); return -1;
	}

	if (common_init (tr) < 0)
		return -1;

	// init reading pipe first (writing pipe gets blocked
	// when it's not opened by the other application)
	if (transport_init_prio_dir (one_way_init, tr, 0) < 0)
		return -1;
	sleep (1);
	if (transport_init_prio_dir (one_way_init, tr, 1) < 0)
		return -1;
	sleep (1);

	return 0;
}

struct washer_config_entry *find_config_entry (
	struct washer_config_list_node *list, const char *type)
{
	struct washer_config_list_node *pos;

	for_each_config_entry (pos, list)
		if (!strcmp (pos->entry.type, type))
			return &pos->entry;
	
	err ("entry not found (%s)", type);
	return NULL;
}

int msg_ok (const char *msg)
{
	size_t len;
	size_t i;
	char c;

	len = strlen (msg);
	for (i = 0; i < len; i ++) {
		c = msg [i];
		if (isalnum (c) || c == ' ')
			continue;

		return 0;
	}

	return 1;
}


// if tr->type == TRANSPORT_IN, then we're sending an ack
int transport_plain_push (struct transport_descriptor *tr, const char *msg)
{
	struct one_way_transport *cur_tr;

	if (!transport_ok (tr))
		return -1;

	if (!msg_ok (msg)) {
		err ("!msg_ok");
		return -1;
	}

	cur_tr = (tr->type == TRANSPORT_OUT) ? &tr->fw : &tr->ack;

	switch (tr->type) {
	case TRANSPORT_FIFO: return transport_push_fifo (cur_tr, msg);
	default: err ("bad transport type"); return -1;
	}
	return 0;
}

// if tr->type == TRANSPORT_OUT, then we're waiting for an ack
int transport_plain_pull (struct transport_descriptor *tr, char *msg)
{
	struct one_way_transport *cur_tr;

	if (!transport_ok (tr))
		return -1;

	cur_tr = (tr->type == TRANSPORT_IN) ? &tr->fw : &tr->ack;

	switch (tr->type) {
	case TRANSPORT_FIFO: return transport_pull_fifo (cur_tr, msg);
	default: err ("bad transport type"); return -1;
	}
	return 0;
}

// sends msg and waits for ack
int transport_push (struct transport_descriptor *tr, const char *msg)
{
	char ack_msg [MAX_MSG_LEN];

	if (transport_plain_push (tr, msg) < 0)
		return -1;
	if (transport_plain_pull (tr, ack_msg) < 0)
		return -1;

	if (strcmp (ack_msg, "OK")) {
		err ("bad ack: %s", ack_msg);
		return -1;
	}
	return 0;
}

// pulls msg and sends ack
int transport_pull (struct transport_descriptor *tr, char *msg)
{
	if (transport_plain_pull (tr, msg) < 0)
		return -1;
	if (transport_plain_push (tr, "OK") < 0)
		return -1;
	
	return 0;
}

