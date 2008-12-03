
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "../../lib/lib.h"
#include "libwasher.h"

#define APP_NAME "libwasher/fifo"

struct washer_msg_buf
{
	long type;
	char_msg_t msg;
};

int transport_init_ipc_common (struct transport_descriptor *tr)
{
	if (creat ("tr-ipc", S_IRUSR | S_IWUSR) < 0) {
		err ("can't create file for ipc");
		return -1;
	}

	return 0;
}

int transport_init_msg_dir (struct one_way_transport *tr, int ack,
	enum TRANSPORT_DIRECTIONS dir)
{
	if ((tr->key = ftok ("tr-ipc", ack ? 2 : 1)) < 0) {
		err ("can't get ipc key");
		return -1;
	}
	if ((tr->msgid = msgget (tr->key, 0666 | IPC_CREAT)) < 0) {
		err ("can't get msg queue");
		return -1;
	}

	return 0;
}

int transport_push_msg (struct one_way_transport *tr, const char *msg)
{
	struct washer_msg_buf buf;

	buf.type = 1;
	strcpy (buf.msg, msg);

	if (msgsnd (tr->msgid, (struct msgbuf *)&buf, strlen (buf.msg) + 1,
		IPC_NOWAIT) < 0) {
		err ("msgsnd failed");
		return -1;
	}

	return 0;
}

int transport_pull_msg (struct one_way_transport *tr, char *msg)
{
	ssize_t res;

	if ((res = msgrcv (tr->msgid, msg, MAX_MSG_LEN, 0, 0)) == -1) {
		err ("msgrcv failed");
		return -1;
	}
	if (msg [res - 1] != '\0') {
		err ("end of msg is not 0");
		return -1;
	}

	return 0;
}

