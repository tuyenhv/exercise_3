#ifndef __TEXT_H
#define __TEXT_H

void insert_char(int c);
void save(void);
void set_status_message (const char *fmt, ...);
void insert_new_line(void);
void insert_row(int at, char *s, size_t len);
#endif
