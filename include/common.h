#ifndef __COMMON_H
#define __COMMON_H

void die(const char *);
void enable_rawmode(void);
struct termios orig_termios;

#endif
