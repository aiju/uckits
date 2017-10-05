TARGET=stm32

include ../Makefile.pre

OFILES=\
	$(patsubst port/%.c,stm32/%.o,$(wildcard port/*.c)) \
	$(patsubst stm32/%.c,stm32/%.o,$(wildcard stm32/*.c)) \
	$(patsubst stm32/%.S,stm32/%.o,$(wildcard stm32/*.S))

.PHONY: all

all: $(UCKITS)/$(TARGET)/lib/libc.a

$(UCKITS)/$(TARGET)/lib/libc.a: $(OFILES)
	ar cr $@ $(OFILES)

stm32/%.o: stm32/%.S
	$(CC) -c $(CFLAGS) $< -o $@

stm32/%.o: port/%.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(OFILES)

.PHONY: nuke
nuke: clean
	$(RM) $(UCKITS)/$(TARGET)/lib/libc.a
