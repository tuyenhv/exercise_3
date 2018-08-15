#ifndef __COMMON_H
#define __COMMON_H

#include <termios.h>
#include <time.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT {NULL, 0}
#define TAB_STOP 8
#define FORCE_QUIT_TIMES 3

typedef struct erow {
  int size;
  int rsize;
  char *chars;
  char *render;
} erow_t;

enum editorKey {
  ENTER = 13,
  INSERT = 45,
  BACK_SPACE = 127,
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
  int rx;
  int rowoff, coloff;
  int screen_rows;
  int screen_cols;
  int num_rows;
  erow_t *row;
  int dirty;
  char *file_name;
  char status_msg[80];
  time_t status_msg_time;
  struct termios orig_termios;
};

struct abuf {
  char *b;
  int len;
};

struct config E;

void ab_append(struct abuf *ab, const char *s, int len);
void die(const char *);
void ab_free(struct abuf *ab);

#endif
