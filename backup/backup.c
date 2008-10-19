
void create_backup (const char *backup, const char *source)
{
	DIR *
}

int main (int argc, char *argv [])
{
	if (argc != 3) {
		printf ("Usage: backup <src-dir> <dest-dir>");
		return 1;
	}

	create_backup (argv [2], argv [1]);

	return 0;
}

