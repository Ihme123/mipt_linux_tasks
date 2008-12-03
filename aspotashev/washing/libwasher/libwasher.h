
#define MAX_MSG_LEN 100

typedef char char_msg_t[MAX_MSG_LEN];

struct washer_config_entry {
	char *type;
	int val;
};

struct washer_config_list_node {
	struct washer_config_list_node *next;
	struct washer_config_entry entry;
};


struct washer_config_list_node *read_configuration (const char *conf_file);

struct washer_config_entry *find_config_entry (
	struct washer_config_list_node *list, const char *type);

#define for_each_config_entry(pos, head) \
	for (pos = head; (pos) != NULL; pos = pos->next)

enum TRANSPORT_TYPES {
	TRANSPORT_FIFO = 1,
	TRANSPORT_MSG  = 2,
};

enum TRANSPORT_DIRECTIONS {
	TRANSPORT_OUT = 1,
	TRANSPORT_IN  = 2,
};

struct one_way_transport
{
	int fd;
};

struct transport_descriptor
{
	enum TRANSPORT_TYPES type;
	enum TRANSPORT_DIRECTIONS dir;

	struct one_way_transport fw;
	struct one_way_transport ack;
};

int transport_init (struct transport_descriptor *transport,
	enum TRANSPORT_TYPES type, enum TRANSPORT_DIRECTIONS dir);

int transport_push (struct transport_descriptor *tr, const char *msg);
int transport_pull (struct transport_descriptor *tr, char *msg);

int get_table_limit ();

int is_sending_transport (enum TRANSPORT_DIRECTIONS dir, int ack);

enum TRANSPORT_TYPES get_tr_type ();

