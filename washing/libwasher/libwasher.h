
struct washer_config_entry {
	char *type;
	int val;
};

struct washer_config_list_node {
	struct washer_config_list_node *next;
	struct washer_config_entry entry;
};

#define for_each_config_entry(pos, head) \
	for (pos = head; (pos) != NULL; pos = pos->next)
