#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../inc/common.h"
#include "../inc/window.h"
#include "../inc/text.h"

extern void enable_rawmode(void);
void process_pressed_key(void);

void ab_append(struct abuf *ab, const char *s, int len) {
  char *new = realloc(ab->b, ab->len + len);
  if (new == NULL) return;
  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}
void ab_free(struct abuf *ab) {
  free(ab->b);
}

/* Init value for E structure */
static void init_editor(void) {
  E.cx = 0;
  E.cy = 0;
  E.rx = 0;
  E.rowoff = 0;
  E.coloff = 0;
  E.num_rows = 0;
  E.row = NULL;
  E.dirty = 0;
  E.file_name = NULL;
  E.status_msg[0] = '\0';
  E.status_msg_time = 0;

  if (get_window_size(&E.screen_rows, &E.screen_cols) == -1) die("get_window_size");
  E.screen_rows -= 2;
}

int main(int argc, char *argv[]) {
  enable_rawmode();
  init_editor();
  if (argc >= 2)
    editor_open(argv[1]);

  set_status_message("HELP: Ctrl-S = save the file | Ctrl-Q = quit the editor");

  while (1) {
    refresh_screen();
    process_pressed_key();
  }
  return 0;
}
