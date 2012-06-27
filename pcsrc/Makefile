CC = gcc
LD = gcc
CFLAGS += -Wall -ggdb
TARGET = gbaxfer
OBJS = main.o xfer.o multi.o gbaencryption.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm $(OBJS) $(TARGET)