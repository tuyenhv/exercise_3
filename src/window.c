#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <ctype.h>
#include "../include/window.h"
#include "../include/common.h"

struct config E;
//extern void ab_append(struct abuf *ab, const char *s, int len);
static void draw_rows(struct abuf *ab) {
  int y;
  for (y = 0; y < E.screen_rows; y++){
    ab_append(ab, "~", 1);

    ab_append(ab, "\x1b[K", 3);
    if (y < E.screen_rows - 1){
      ab_append(ab, "\r\n", 2);
    }
  }
}

/* clear the screen */
void refresh_screen(void) {
  struct abuf ab = ABUF_INIT;

  ab_append(&ab, "\x1b[?25l", 6);
  ab_append(&ab, "\x1b[H", 3);

  draw_rows(&ab);

  ab_append(&ab, "\x1b[H", 3);
  ab_append(&ab, "\x1b[?25h", 6);

  write(STDOUT_FILENO, ab.b, ab.len);
  ab_free(&ab);
}


int get_cursor_position(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  buf[i] = '\0';
  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

  return 0;
}

int get_window_size(int *rows, int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    return get_cursor_position(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

void init_editor(void) {
  if (get_window_size(&E.screen_rows, &E.screen_cols) == -1) die("get_window_size");
}
