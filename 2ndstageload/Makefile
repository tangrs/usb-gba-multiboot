HOSTCC = gcc
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
GBAFIX = $(DEVKITARM)/bin/gbafix
CFLAGS += -Wall -nostdlib
TARGETS = loader.gba

all: $(TARGETS) $(TARGETS:.gba=.h)

bin2c: bin2c.c
	$(HOSTCC) $< -o $@

%.h: %.gba bin2c
	./bin2c $< > $@

%.gba: %.elf
	$(OBJCOPY) -O binary $< $@
	$(GBAFIX) -p $@

%.elf: %.s
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS) $(TARGETS:.gba=.h)