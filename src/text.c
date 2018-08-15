#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include "../inc/common.h"
#include "../inc/window.h"
#include "../inc/text.h"

extern bool insert_flag;
extern int read_key(void);

static void update_row(erow_t *row) {
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

/* Insert a charater to at position on a row. */
static void row_insert_char(erow_t *row, int at, int c) {
  if (at < 0 || at > row->size - 1)
    at = row->size;
  row->chars = realloc(row->chars, row->size + 2);
  /* Move content from at posititon and after it to at + 1 position,
   * then insert the c character to at position. */
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  row->size++;
  row->chars[at] = c;
  update_row(row);
  E.dirty++;
}

static void row_overwrite_char(erow_t *row, int at, int c) {
  /* New line or at is at the end of line. */
  if (at < 0 || at >= row->size){
    at = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    row->size++;
  }
  row->chars[at] = c;
  update_row(row);
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

static void free_row(erow_t *row) {
  free(row->render);
  free(row->chars);
}

static void row_del_char(erow_t *row, int at) {
  if (at < 0 || at >= row->size) return;
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  update_row(row);
  E.dirty++;
}

void del_row(int at) {
  if (at < 0 || at >= E.num_rows) return;
  free_row(&E.row[at]);
  memmove(&E.row[at], &E.row[at + 1], sizeof(erow_t) * (E.num_rows - at - 1));
  E.num_rows--;
  E.dirty++;
}

void del_char(void) {
  if (E.cy == E.num_rows) return;
  if (E.cx == 0 && E.cy == 0) return;

  erow_t *row = &E.row[E.cy];
  if (E.cx > 0) {
    row_del_char(row, E.cx - 1);
    E.cx--;
  } else {
    /* If the cursor is at the begging of line, append the current line to the
     * before line. */
    E.cx = E.row[E.cy - 1].size;
    row_append_string(&E.row[E.cy - 1], row->chars, row->size);
    del_row(E.cy);
    E.cy--;
  }
}

void insert_char(int c) {
  if (E.cy == E.num_rows) {
    insert_row(E.num_rows, "", 0);
  }
  if (insert_flag)
    row_insert_char(&E.row[E.cy], E.cx, c);
  else
    row_overwrite_char(&E.row[E.cy], E.cx, c);
  E.cx++;
}

void insert_new_line(void) {
  if (E.cx == 0) {
    insert_row(E.cy, "", 0);
  } else {
    erow_t *row = &E.row[E.cy];
    insert_row(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
    row = &E.row[E.cy];
    row->size = E.cx;
    row->chars[row->size] = '\0';
    update_row(row);
  }
  E.cy++;
  E.cx = 0;
}

/* Copy all content of the file to buffer. */
static char *row_to_string(int *buf_len) {
  int tot_len = 0;
  int j;
  for (j = 0; j < E.num_rows; j++) {
    tot_len += E.row[j].size + 1;
  }
  *buf_len = tot_len;

  char *buf = malloc(tot_len);
  char *p = buf;
  for (j = 0; j < E.num_rows; j++) {
    memcpy(p, E.row[j].chars, E.row[j].size);
    p += E.row[j].size;
    *p = '\n';
    p++;
  }

  return buf;
}

void set_status_message (const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(E.status_msg, sizeof(E.status_msg), fmt, ap);
  va_end(ap);
  E.status_msg_time = time(NULL);
}

static char *prompt(char *prompt) {
  size_t buf_size = 128;
  char *buf = malloc(buf_size);

  size_t buf_len = 0;
  buf[0] = '\0';

  while (1) {
    set_status_message(prompt, buf);
    refresh_screen();

    int c = read_key();
    if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACK_SPACE) {
      if (buf_len != 0)
        buf[--buf_len] = '\0';
    } else if (c == '\x1b') {
      /* Press Esc to cancel input prompt. */
      set_status_message("");
      free(buf);
      return NULL;
    } else if (c == '\r') {
      if (buf_len != 0) {
        set_status_message("");
        return buf;
      }
    }else if (!iscntrl(c) && c < 128) {
      if (buf_len == buf_size - 1) {
        buf_size *= 2;
        buf = realloc(buf, buf_size);
      }
      buf[buf_len++] = c;
      buf[buf_len] = '\0';
    }
  }
}

void save(void) {
  if (E.file_name == NULL){
    E.file_name = prompt("Save as: %s (ESC to cancel)");
    if (E.file_name == NULL) {
      set_status_message("save aborted");
      return;
    }
  }

  int len;
  char *buf = row_to_string(&len);

  int fd = open(E.file_name, O_RDWR | O_CREAT, 0644);
  if (fd != -1) {
    if (ftruncate(fd, len) != -1) {
      if (write(fd, buf, len) == len) {
        close(fd);
        free(buf);
        E.dirty = 0;
        set_status_message("%d bytes written to disk", len);
        return;
      }
    }
    close(fd);
  }

  free(buf);
  set_status_message("Cannot save the file! Error: %s", strerror(errno));
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