#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "../include/common.h"
#include "../include/window.h"

extern void init_editor(void);
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

int main() {
  enable_rawmode();
  init_editor();
  while (1) {
    refresh_screen();
    process_pressed_key();
  }
  return 0;
}
