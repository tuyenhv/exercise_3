#ifndef __WINDOW_H
#define __WINDOW_H

#include "common.h"

void refresh_screen(void);
void editor_open(char *filename);
void update_row(erow_t *row);
void insert_row(int at, char *s, size_t len);
int get_window_size(int *rows, int *cols);

#endif
