#ifndef __COMMON_H
#define __COMMON_H

#include <termios.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT {NULL, 0}

struct config {
  int screen_rows;
  int screen_cols;
  struct termios orig_termios;
};

struct abuf {
  char *b;
  int len;
};

extern struct config E;

void ab_append(struct abuf *ab, const char *s, int len);
void die(const char *);
void enable_rawmode(void);
void ab_free(struct abuf *ab);

#endif
