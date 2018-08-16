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

    /* Clear screen */
    ab_append(ab, "\x1b[K", 3);
    ab_append(ab, "\r\n", 2);
  }
}

/* Handle tab character. */
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

static void draw_message_bar(struct abuf *ab) {
  ab_append(ab, "\x1b[K", 3);
  int msg_len = strlen(E.status_msg);
  if (msg_len > E.screen_cols)
    msg_len = E.screen_cols;
  if (msg_len && time(NULL) - E.status_msg_time < 5)
    ab_append(ab, E.status_msg, msg_len);
}

/* update the screen */
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