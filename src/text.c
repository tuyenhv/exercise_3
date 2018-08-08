#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include "../inc/common.h"
#include "../inc/window.h"

static void row_insert_char(erow_t *row, int at, int c) {
  if (at < 0 || at > row->size)
    at = row->size;
  row->chars = realloc(row->chars, row->size + 2);
  memmove(&row->chars[at + 1], &row->chars[at], row->size -at + 1);
  row->size++;
  row->chars[at] = c;
  update_row(row);
}

void insert_char(int c) {
  if (E.cy == E.num_rows) {
    append_row("", 0);
  }
  row_insert_char(&E.row[E.cy], E.cx, c);
  E.cx++;
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
        return;
      }
    }
    close(fd);
  }

  free(buf);
}

void set_status_message (const char *fmt, ...) {
  va_list ap;
  var_start(ap, fmt);
  vsnprintf(E.status_msg, sizeof(E.status_msg), fmt, ap);
  va_end(ap);
  E.status_msg_time = time(NULL);
}
