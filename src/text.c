#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include "../inc/common.h"
#include "../inc/window.h"

extern void row_append_string(erow_t *row, char *s, size_t len);
extern void del_row(int at);
static void row_insert_char(erow_t *row, int at, int c) {
  if (at < 0 || at > row->size)
    at = row->size;
  row->chars = realloc(row->chars, row->size + 2);
  memmove(&row->chars[at + 1], &row->chars[at], row->size -at + 1);
  row->size++;
  row->chars[at] = c;
  update_row(row);
  E.dirty++;
}

static void row_del_char(erow_t *row, int at) {
  if (at < 0 || at >= row->size) return;
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
  update_row(row);
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
  row_insert_char(&E.row[E.cy], E.cx, c);
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

char *row_to_string(int *buf_len) {
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

void save(void) {
  if (E.file_name == NULL)
    return;

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
