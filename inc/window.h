#ifndef __WINDOW_H
#define __WINDOW_H

#include "common.h"

void refresh_screen(void);
void init_editor(void);
void editor_open(char *filename);
void update_row(erow_t *row);
void append_row(char *s, size_t len);

#endif