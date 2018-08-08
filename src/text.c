#include <stdlib.h>
#include <string.h>
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
