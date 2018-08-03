#ifndef __COMMON_H
#define __COMMON_H

#include <termios.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT {NULL, 0}

typedef struct erow {
  int size;
  char *chars;
} erow_t;

enum editorKey {
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

struct config {
  int cx, cy;
  int screen_rows;
  int screen_cols;
  int num_rows;
  erow_t *row;
  struct termios orig_termios;
};

struct abuf {
  char *b;
  int len;
};

extern struct config E;

void ab_append(struct abuf *ab, const char *s, int len);
void die(const char *);
void ab_free(struct abuf *ab);

#endif
