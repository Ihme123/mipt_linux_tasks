libwasher.so: libwasher.o config.o fifo.o msg.o
	$(CC) -g -shared -o libwasher.so libwasher.o config.o fifo.o msg.o
config.o: config.c
	$(CC) -Wall -g -c -fPIC -o config.o config.c
fifo.o: fifo.c
	$(CC) -Wall -g -c -fPIC -o fifo.o fifo.c
msg.o: msg.c
	$(CC) -Wall -g -c -fPIC -o msg.o msg.c
libwasher.o: libwasher.c
	$(CC) -Wall -g -c -fPIC -o libwasher.o libwasher.c
