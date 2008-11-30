libwasher.so: libwasher.o config.o
	$(CC) -g -shared -o libwasher.so libwasher.o config.o
config.o: config.c
	$(CC) -Wall -g -c -fPIC -o config.o config.c
libwasher.o: libwasher.c
	$(CC) -Wall -g -c -fPIC -o libwasher.o libwasher.c

