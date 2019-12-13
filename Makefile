CC = gcc

CFLAGS += -Wall -fno-stack-protector -Wextra -g

.PHONY: all clean

# Build
all: release

release: serverRelease clientRelease

serverRelease:
	$(CC) $(CFLAGS) server.c -o server $(LIBS)

clientRelease:
	$(CC) $(CFLAGS) client.c -o client $(LIBS)

clean:
	-rm -f server client
