all:
	$(MAKE) -C libwasher --no-print-directory
	$(MAKE) -C washer --no-print-directory
	$(MAKE) -C drier --no-print-directory

clean:
	rm libwasher/libwasher.o libwasher/libwasher.so \
		washer/washer.o washer/washer \
		drier/drier.o drier/drier
