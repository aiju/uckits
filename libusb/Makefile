TARGET=stm32

include ../Makefile.pre

OFILES:=\
	$(patsubst port/%.c,stm32/%.o,$(wildcard port/*.c)) \
	$(patsubst port/%.desc,stm32/%.tab.o,$(wildcard port/*.desc)) \

TABFILES:=\
	$(patsubst port/%.desc,port/%.tab.c,$(wildcard port/*.desc)) \

.PHONY: all

all: $(UCKITS)/gcc/bin/mkdesc $(UCKITS)/$(TARGET)/lib/libusb.a

$(UCKITS)/gcc/bin/mkdesc: mkdesc.c
	gcc -Wall -Wno-unused-function -Wno-parentheses -fno-diagnostics-show-caret $< -o $@

$(UCKITS)/$(TARGET)/lib/libusb.a: $(OFILES)
	ar cr $@ $(OFILES)

stm32/%.o: stm32/%.c
	$(CC) -c $(CFLAGS) $< -o $@

stm32/%.o: stm32/%.S
	$(CC) -c $(CFLAGS) $< -o $@

stm32/%.o: port/%.c
	$(CC) -c $(CFLAGS) $< -o $@

stm32/%.o: fmt/%.c
	$(CC) -c $(CFLAGS) $< -o $@

port/%.tab.c: port/%.desc $(UCKITS)/gcc/bin/mkdesc
	mkdesc $< $* > $@

.PHONY: clean
clean:
	$(RM) $(OFILES) $(TABFILES)

.PHONY: nuke
nuke: clean
	$(RM) $(UCKITS)/$(TARGET)/lib/libusb.a
