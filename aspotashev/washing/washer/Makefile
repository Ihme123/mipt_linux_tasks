all: washer
washer.o: washer.c ../libwasher/libwasher.h
	$(CC) -c -Wall -g washer.c -o washer.o
washer: washer.o
	$(CC) -Wall -g -L../libwasher -lwasher washer.o -o washer
