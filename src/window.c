#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include "../include/window.h"
#include "../include/common.h"

struct config E;
//extern void ab_append(struct abuf *ab, const char *s, int len);

/* Draw a column of tildes (~) on the left hand side of the screen */
static void draw_rows(struct abuf *ab) {
  int y;
  for (y = 0; y < E.screen_rows; y++) {
    if (y >= E.num_rows) {
      if (E.num_rows == 0 && y == E.screen_rows / 3) {
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome),
          "Welcome to the minimal editor");
        if (welcomelen > E.screen_cols) welcomelen = E.screen_cols;
        int padding = (E.screen_cols - welcomelen) / 2;
        if (padding) {
          ab_append(ab, "~", 1);
          padding--;
        }
        while (padding--) ab_append(ab, " ", 1);
        ab_append(ab, welcome, welcomelen);
      } else {
        ab_append(ab, "~", 1);
      }
    } else {
      int len = E.row[y].size;
      if (len > E.screen_cols)
        len = E.screen_cols;
      ab_append(ab, E.row[y].chars, len);
    }

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
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
  ab_append(&ab, buf, strlen(buf));

  ab_append(&ab, "\x1b[?25h", 6);

  write(STDOUT_FILENO, ab.b, ab.len);
  ab_free(&ab);
}

static int get_cursor_position(int *rows, int *cols) {
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

/* Get the size of the terminal */
static int get_window_size(int *rows, int *cols) {
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
  E.cx = 0;
  E.cy = 0;
  E.num_rows = 0;
  E.row = NULL;

  if (get_window_size(&E.screen_rows, &E.screen_cols) == -1) die("get_window_size");
}

static void append_row(char *s, size_t len) {
  E.row = realloc(E.row, sizeof(erow_t) * (E.num_rows + 1));
  int at = E.num_rows;
  E.row[at].size = len;
  E.row[at].chars = malloc(len + 1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';
  E.num_rows++;
}

void editor_open(char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp) die("fopen: Cannot open the file");

  char *line = NULL;
  size_t line_cap = 0;
  ssize_t line_len;
  while ((line_len = getline(&line, &line_cap, fp)) != -1) {
    while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
      line_len--;
    }
    append_row(line, line_len);
  }
  free(line);
  fclose(fp);
}
