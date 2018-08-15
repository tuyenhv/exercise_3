#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include "../inc/window.h"
#include "../inc/common.h"

/* Draw a column of tildes (~) on the left hand side of the screen */
static void draw_rows(struct abuf *ab) {
  int y;
  for (y = 0; y < E.screen_rows; y++) {
    int file_row = y + E.rowoff;
    if (file_row >= E.num_rows) {
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
      int len = E.row[file_row].rsize - E.coloff;
      if (len < 0)
        len = 0;
      if (len > E.screen_cols)
        len = E.screen_cols;
      ab_append(ab, &E.row[file_row].render[E.coloff], len);
    }

    ab_append(ab, "\x1b[K", 3);
    ab_append(ab, "\r\n", 2);
  }
}

static int row_cx_to_rx (erow_t *row, int cx) {
  int rx = 0;
  int j;
  for (j = 0; j < cx; j++) {
    if (row->chars[j] == '\t')
      rx += (TAB_STOP - 1) - (rx % TAB_STOP);
    rx++;
  }
  return rx;
}

static void scroll(void){
  E.rx = 0;
  if (E.cy < E.num_rows) {
    E.rx = row_cx_to_rx(&E.row[E.cy], E.cx);
  }

  if (E.cy < E.rowoff) {
    E.rowoff = E.cy;
  }
  if (E.cy >= E.rowoff + E.screen_rows) {
    E.rowoff = E.cy - E.screen_rows + 1;
  }
  if (E.rx < E.coloff) {
    E.coloff = E.rx;
  }
  if (E.rx < E.coloff) {
    E.coloff = E.rx;
  }
  if (E.rx >= E.coloff + E.screen_cols) {
    E.coloff = E.rx - E.screen_cols + 1;
  }
}

static void draw_status_bar(struct abuf *ab) {
  ab_append(ab, "\x1b[7m", 4);
  char status[80], rstatus[80];
  int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
    E.file_name ? E.file_name : "[No Name]", E.num_rows,
    E.dirty ? "(modified)" : "");
  int rlen = snprintf(rstatus, sizeof(rstatus), "%d%d",
    E.cy + 1, E.num_rows);

  if (len > E.screen_rows)
    len = E.screen_rows;
  ab_append(ab, status, len);

  while (len < E.screen_cols) {
    if (E.screen_cols - len == rlen) {
      ab_append(ab, rstatus, rlen);
      break;
    } else {
      ab_append(ab, " ", 1);
      len++;
    }
  }
  ab_append(ab, "\x1b[m", 3);
  ab_append(ab, "\r\n", 2);
}

void draw_message_bar(struct abuf *ab) {
  ab_append(ab, "\x1b[K", 3);
  int msg_len = strlen(E.status_msg);
  if (msg_len > E.screen_cols)
    msg_len = E.screen_cols;
  if (msg_len && time(NULL) - E.status_msg_time < 5)
    ab_append(ab, E.status_msg, msg_len);
}

/* clear the screen */
void refresh_screen(void) {
  scroll();

  struct abuf ab = ABUF_INIT;

  ab_append(&ab, "\x1b[?25l", 6);
  ab_append(&ab, "\x1b[H", 3);

  draw_rows(&ab);
  draw_status_bar(&ab);
  draw_message_bar(&ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy - E.rowoff + 1, E.rx - E.coloff + 1);
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

void update_row(erow_t *row) {
  int tabs = 0;
  int j;
  for (j = 0; j < row->size; j++)
    if (row->chars[j] == '\t') tabs++;

  free(row->render);
  row->render = malloc((row->size) + tabs * (TAB_STOP - 1) + 1);

  int idx = 0;
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      row->render[idx++] = ' ';
      while (idx % TAB_STOP != 0) {
        row->render[idx++] = ' ';
      }
    } else {
      row->render[idx++] = row->chars[j];
    }
  }
  row->render[idx] = '\0';
  row->rsize = idx;
}

/* Insert strings to a line at in the file, length of line is len, pointer from s.*/
void insert_row(int at, char *s, size_t len) {
  if (at < 0 || at > E.num_rows) return;

  E.row = realloc(E.row, sizeof(erow_t) * (E.num_rows + 1));
  memmove(&E.row[at + 1], &E.row[at], sizeof(erow_t) * (E.num_rows - at));

  E.row[at].size = len;
  E.row[at].chars = malloc(len + 1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';

  E.row[at].rsize = 0;
  E.row[at].render = NULL;
  update_row(&E.row[at]);

  E.num_rows++;
  E.dirty++;
}

static void free_row(erow_t *row) {
  free(row->render);
  free(row->chars);
}

void del_row(int at) {
  if (at < 0 || at >= E.num_rows) return;
  free_row(&E.row[at]);
  memmove(&E.row[at], &E.row[at + 1], sizeof(erow_t) * (E.num_rows - at - 1));
  E.num_rows--;
  E.dirty++;
}

void row_append_string(erow_t *row, char *s, size_t len) {
  row->chars = realloc(row->chars, row->size + len + 1);
  memcpy(&row->chars[row->size], s, len);
  row->size += len;
  row->chars[row->size] = '\0';
  update_row(row);
  E.dirty++;
}

