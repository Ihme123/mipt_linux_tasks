
int transport_init_fifo_common (struct transport_descriptor *tr);
int transport_init_fifo_dir (struct one_way_transport *tr,
	int ack, enum TRANSPORT_DIRECTIONS dir);

int transport_push_fifo (struct one_way_transport *tr,
	const char *msg);
int transport_pull_fifo (struct one_way_transport *tr,
	char *msg);

