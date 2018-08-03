#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "../inc/common.h"

static int read_key(void) {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }

  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '1': return HOME_KEY;
            case '3': return DEL_KEY;
            case '4': return END_KEY;
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
            case '7': return HOME_KEY;
            case '8': return END_KEY;
          }
        }
      } else {
        switch (seq[1]) {
          case 'A': return ARROW_UP;
          case 'B': return ARROW_DOWN;
          case 'C': return ARROW_RIGHT;
          case 'D': return ARROW_LEFT;
          case 'H': return HOME_KEY;
          case 'F': return END_KEY;
        }
      }
    } else if (seq[0] == '0') {
      switch (seq[1]) {
        case 'H': return HOME_KEY;
        case 'F': return END_KEY;
      }
    }

    return '\x1b';
  } else {
    return c;
  }
}

/* Allow user move cursor using keys. */
static void move_cursor(int key) {
  switch (key) {
    case ARROW_LEFT:
      if (E.cx != 0)
        E.cx--;
      break;
    case ARROW_RIGHT:
      if (E.cx != E.screen_cols - 1)
        E.cx++;
      break;
    case ARROW_UP:
      if (E.cy != 0)
        E.cy--;
      break;
    case ARROW_DOWN:
      if (E.cy != E.screen_rows - 1)
        E.cy++;
      break;
  }
}

void process_pressed_key(void) {
  int c = read_key();

  switch (c) {
    case CTRL_KEY('q'):
      /* Clear the screen before exitting */
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);

      exit(0);
      break;

    case HOME_KEY:
      E.cx = 0;
      break;

    case END_KEY:
      E.cx = E.screen_cols - 1;
      break;

    case PAGE_UP:
    case PAGE_DOWN:
      {
        int times = E.screen_rows;
        while (times--) {
          move_cursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
        }
      }
      break;
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
      move_cursor(c);
      break;
  }
}
