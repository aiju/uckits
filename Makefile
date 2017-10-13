DIRS=libc libio libusb

.PHONY: all clean nuke
all clean nuke:
	for i in $(DIRS); do make -C $$i $@; done
