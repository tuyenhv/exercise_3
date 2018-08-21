sources = src/common.c src/terminal_modes.c src/key.c src/window.c src/editor.c \
					src/text.c
CC = gcc

CFLAGS = -Wall -Wno-unused-result

ifndef DEBUG
	CFLAGS	+= -O2
else
	CFLAGS	+= -O0 -g
endif

all:
	$(CC) $(CFLAGS) $(sources) -o editor
clean:
	rm -f editor src/*.o *.o
