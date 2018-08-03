#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/common.h"

static char read_key(void) {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

// Check the function again
void process_pressed_key(void) {
  char c = read_key();
  printf("%c \n", c);
  switch (c) {
    case CTRL_KEY('q'):
      /* Clear the screen before exitting */
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);

      exit(0);
      break;
    default :
      printf("%c \n", c);
  }
}
