sources = src/editor.c src/terminal_modes.c
CC = gcc

CFLAGS = -Wall
ifndef DEBUG
	CFLAGS	+= -O2
else
	CFLAGS	+= -O0 -g
endif

all:
	$(CC) $(CFLAGS) $(sources) -o editor
clean:
	rm -f editor src/*.o *.o
