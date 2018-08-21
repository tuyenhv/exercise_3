#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../inc/common.h"
#include "../inc/text.h"

extern void del_char(void);
extern void find(void);

bool insert_flag = true;

int read_key(void) {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }

  /*
  Config is pretty much default:

  key_bindings:
  - { key: V,        mods: Shift|Control, action: Paste                        }
  - { key: C,        mods: Shift|Control, action: Copy                         }
  - { key: Home,                    chars: "\x1b[H",   mode: ~AppCursor  }
  - { key: Home,                    chars: "\x1b[1~",  mode: AppCursor   }
  - { key: End,                     chars: "\x1b[F",   mode: ~AppCursor  }
  - { key: End,                     chars: "\x1b[4~",  mode: AppCursor   }
  - { key: PageUp,                  chars: "\x1b[5~"                     }
  - { key: PageDown,                chars: "\x1b[6~"                     }
  - { key: Left,     mods: Shift,   chars: "\x1b[1;2D"                   }
  - { key: Left,     mods: Control, chars: "\x1b[1;5D"                   }
  - { key: Left,     mods: Alt,     chars: "\x1b[1;3D"                   }
  - { key: Left,                    chars: "\x1b[D",   mode: ~AppCursor  }
  - { key: Left,                    chars: "\x1bOD",   mode: AppCursor   }
  - { key: Right,    mods: Shift,   chars: "\x1b[1;2C"                   }
  - { key: Right,    mods: Control, chars: "\x1b[1;5C"                   }
  - { key: Right,    mods: Alt,     chars: "\x1b[1;3C"                   }
  - { key: Right,                   chars: "\x1b[C",   mode: ~AppCursor  }
  - { key: Right,                   chars: "\x1bOC",   mode: AppCursor   }
  - { key: Up,       mods: Shift,   chars: "\x1b[1;2A"                   }
  - { key: Up,       mods: Control, chars: "\x1b[1;5A"                   }
  - { key: Up,       mods: Alt,     chars: "\x1b[1;3A"                   }
  - { key: Up,                      chars: "\x1b[A",   mode: ~AppCursor  }
  - { key: Up,                      chars: "\x1bOA",   mode: AppCursor   }
  - { key: Down,     mods: Shift,   chars: "\x1b[1;2B"                   }
  - { key: Down,     mods: Control, chars: "\x1b[1;5B"                   }
  - { key: Down,     mods: Alt,     chars: "\x1b[1;3B"                   }
  - { key: Down,                    chars: "\x1b[B",   mode: ~AppCursor  }
  - { key: Down,                    chars: "\x1bOB",   mode: AppCursor   }
  - { key: F1,                      chars: "\x1bOP"                      }
  - { key: F2,                      chars: "\x1bOQ"                      }
  - { key: F3,                      chars: "\x1bOR"                      }
  - { key: F4,                      chars: "\x1bOS"                      }
  - { key: F5,                      chars: "\x1b[15~"                    }
  - { key: F6,                      chars: "\x1b[17~"                    }
  - { key: F7,                      chars: "\x1b[18~"                    }
  - { key: F8,                      chars: "\x1b[19~"                    }
  - { key: F9,                      chars: "\x1b[20~"                    }
  - { key: F10,                     chars: "\x1b[21~"                    }
  - { key: F11,                     chars: "\x1b[23~"                    }
  - { key: F12,                     chars: "\x1b[24~"                    }
  - { key: Back,                    chars: "\x7f"                        }
  - { key: Delete,                  chars: "\x1b[3~",  mode: AppKeypad   }
  - { key: Delete,                  chars: "\x1b[P",   mode: ~AppKeypad  }
  */
    
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

    case CTRL_KEY('f'):
      find();
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
