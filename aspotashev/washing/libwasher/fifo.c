
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../../lib/lib.h"
#include "libwasher.h"

#define APP_NAME "libwasher/fifo"


int transport_init_fifo_common (struct transport_descriptor *tr)
{
	if (mkfifo ("tr-fifo-ack", S_IWUSR | S_IRUSR) < 0) {
		if (errno == EEXIST)
			info ("fifo already exists");
		else {
			err ("can't create fifo");
			return -1;
		}
	}

	if (mkfifo ("tr-fifo", S_IWUSR | S_IRUSR) < 0) {
		if (errno == EEXIST)
			info ("fifo already exists");
		else {
			err ("can't create fifo");
			return -1;
		}
	}

	return 0;
}

int transport_init_fifo_dir (struct one_way_transport *tr, int ack, enum TRANSPORT_DIRECTIONS dir)
{ // TODO: merge transport_init_fifo to transport_init_fifo_dir
	const char *filename = ack ? "tr-fifo-ack" : "tr-fifo";
	int send;

	send = is_sending_transport (dir, ack);

	info ("opening fifo (%s, ack = %d, dir = %d, send = %d)", filename, ack, dir, send);
	if ((tr->fd = open (filename,
		send ? O_WRONLY : (O_RDONLY))) < 0) {
		err ("can't open fifo");
		return -1;
	}
	info ("fifo opened");

	return 0;
}

int transport_push_fifo (struct one_way_transport *tr,
	const char *msg)
{
	size_t len;
	ssize_t res;
	char *buffer;
	size_t buffer_len;

	len = strlen (msg);
	buffer = malloc ((len + 20) * sizeof (char));
	if (!buffer) {
		err ("can't alloc buffer");
		return -1;
	}

	strcpy (buffer, msg);
	strcat (buffer, "\n");

	buffer_len = strlen (buffer);
	res = write (tr->fd, buffer, buffer_len);
	free (buffer);
	if (res < 0) {
		err ("can't write to fifo");
		return -1;
	} else if (res != buffer_len) {
		err ("writing to fifo: partial success");
		return -1;
	}

	return 0;
}

int transport_pull_fifo (struct one_way_transport *tr, char *msg)
{
	size_t pos;
	char ch;
	ssize_t read_res;

	for (pos = 0; ; pos ++) {
		read_res = read (tr->fd, &ch, 1);
		if (read_res != 1) {
			err ("can't read char from pipe: result: %d", (int)read_res);
			return -1;
		}

		if (ch == '\n')
			break;
		else if (ch == '\0') {
			err ("ack fifo closed unexpectedly");
			return -1;
		}

		msg [pos] = ch;
		info ("symbol read from fifo = [%c]/0x%02x", ch, ch);

		if (pos > MAX_MSG_LEN) {
			err ("msg is too long (fifo)");
			return -1;
		}
	}
	msg [pos] = '\0';

	return 0;
}

