
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
	TRANSPORT_FIFO = 1
};

enum TRANSPORT_DIRECTIONS {
	TRANSPORT_OUT = 1,
	TRANSPORT_IN = 2
};

struct transport_descriptor
{
	enum TRANSPORT_TYPES type;
	enum TRANSPORT_DIRECTIONS dir;

	int fd;
};

int transport_init (struct transport_descriptor *transport,
	enum TRANSPORT_TYPES type, enum TRANSPORT_DIRECTIONS dir);

int transport_push (struct transport_descriptor *tr, const char *msg);

