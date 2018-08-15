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

/* Start opening and editting the file.*/
void editor_open(char *file_name) {
  free(E.file_name);
  /* strdup returns a pointer to a null-terminated byte string. The memory obtained is done
   * dynamically using malloc and it can be freed using free(). */
  E.file_name = strdup(file_name);
  FILE *fp = fopen(file_name, "r");
  /* Fix this:
   * If file_name does not exist, print a error and exit. 
   * This is quite weird and we should open a new file instead of. */
  if (!fp) die("fopen: Cannot open the file");

  /* Refer man-pan of getline() for more detail of implementing like below. */
  char *line = NULL;
  size_t line_cap = 0;
  ssize_t line_len;
  while ((line_len = getline(&line, &line_cap, fp)) != -1) {
    while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
      line_len--;
    }
    insert_row(E.num_rows, line, line_len);
  }
  free(line);
  fclose(fp);
  E.dirty = 0;
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
