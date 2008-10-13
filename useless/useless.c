
#define MAX_STRING_LEN

struct schedule_item {
	int time;
	char cmd [MAX_STRING_LEN];
};

struct schedule_item *read_config (const char *filename)
{
	FILE *conf = fopen ("useless.conf", "r");
	fclose (conf);
}

int main ()
{
	return 0;
}

