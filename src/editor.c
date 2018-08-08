#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../inc/common.h"
#include "../inc/window.h"

void init_editor(void);
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

int main(int argc, char *argv[]) {
  enable_rawmode();
  init_editor();
  if (argc >= 2)
    editor_open(argv[1]);

  set_status_message("HELP: Ctrl-Q = quit");
  
  while (1) {
    refresh_screen();
    process_pressed_key();
  }
  return 0;
}
