libwasher.so: libwasher.o config.o fifo.o
	$(CC) -g -shared -o libwasher.so libwasher.o config.o fifo.o
config.o: config.c
	$(CC) -Wall -g -c -fPIC -o config.o config.c
fifo.o: fifo.c
	$(CC) -Wall -g -c -fPIC -o fifo.o fifo.c
libwasher.o: libwasher.c
	$(CC) -Wall -g -c -fPIC -o libwasher.o libwasher.c

