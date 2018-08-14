#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../inc/common.h"
#include "../inc/text.h"

extern void del_char(void);
bool insert_flag = true;

int read_key(void) {
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
            case '2': return INSERT;
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
  erow_t *row = (E.cy >= E.num_rows) ? NULL : &E.row[E.cy];

  switch (key) {
    case ARROW_LEFT:
      if (E.cx != 0)
        E.cx--;
      else if (E.cy > 0) {
        E.cy--;
        E.cx = E.row[E.cy].size;
      }
      break;
    case ARROW_RIGHT:
      if (row && E.cx < row->size)
        E.cx++;
      else if (row && E.cx == row->size) {
        E.cy++;
        E.cx = 0;
      }
      break;
    case ARROW_UP:
      if (E.cy != 0)
        E.cy--;
      break;
    case ARROW_DOWN:
      if (E.cy < E.num_rows)
        E.cy++;
      break;
  }

  row = (E.cy >= E.num_rows) ? NULL : &E.row[E.cy];
  int rowlen = row ? row->size : 0;
  if (E.cx > rowlen)
    E.cx =rowlen;
}

void process_pressed_key(void) {
  static int quit_times = FORCE_QUIT_TIMES;
  int c = read_key();
  switch (c) {
    case INSERT:
      insert_flag = !insert_flag;
      break;

    case ENTER:
      insert_new_line();
      break;

    case CTRL_KEY('q'):
      if (E.dirty && quit_times > 0) {
        set_status_message("WARNING!!! File has unsaved changes. "
          "Press Ctrl-Q %d more times to quit.", quit_times);
        quit_times--;
        return;
      }
      /* Clear the screen before exitting */
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);

      exit(0);
      break;

    case CTRL_KEY('s'):
      save();
      break;

    case HOME_KEY:
      E.cx = 0;
      break;

    case END_KEY:
      if (E.cy < E.num_rows)
        E.cx = E.row[E.cy].size;
      break;

    case BACK_SPACE:
    case CTRL_KEY('h'):
    case DEL_KEY:
      if (c == DEL_KEY)
        move_cursor(ARROW_RIGHT);
      del_char();
      break;

    case PAGE_UP:
    case PAGE_DOWN:
      {
        if (c == PAGE_UP) {
          E.cy = E.rowoff;
        } else if (c == PAGE_DOWN) {
          E.cy = E.rowoff + E.screen_rows - 1;
          if (E.cy > E.num_rows) E.cy = E.num_rows;
        }

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

    case CTRL_KEY('l'):
    case '\x1b':
      break;

    default:
      insert_char(c);
      break;
  }

  quit_times = FORCE_QUIT_TIMES;
}
