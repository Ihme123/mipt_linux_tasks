all: drier
drier.o: drier.c ../libwasher/libwasher.h
	$(CC) -c -Wall -g drier.c -o drier.o
drier: drier.o
	$(CC) -Wall -g -L../libwasher -lwasher -lpthread drier.o -o drier
