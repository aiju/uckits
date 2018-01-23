CFLAGS=-Wall -Wno-parentheses -Wno-missing-braces -Wno-pointer-to-int-cast -g
LDFLAGS=-Wl,--kill-at -g
OFILES=\
	misc.o \
	usb.o \
	brd.o \
	dev.o \
	napi.o \
	md5.o

TARG=ni4882.dll

HFILES=../usbif.h dll.h dat.h fns.h

all: $(TARG)

$(TARG): $(OFILES)
	$(CC) $(LDFLAGS) -shared $(OFILES) -lsetupapi -lwinusb -o $@

%.o: %.c $(HFILES)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARG) $(OFILES)
