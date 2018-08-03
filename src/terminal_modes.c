#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "../include/common.h"
#include "../include/terminal_modes.h"

void disable_rawmode(void){
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}

void enable_rawmode(void){
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
  atexit(disable_rawmode);
  struct termios raw = E.orig_termios;
  
  /* Explanation for features with each flag: **********************************
   * The below is flags and the meaning will change from on to off if we change
   * the value of flags.
   * ECHO: Print what you typed to the terminal.
   * ICANON: Allow us to turn off canonical mode. (read input byte by byte instead
   *         of line by line).
   * ISIG: Turn off/on the sending of both of these signals. (SIGINT and SIGTSTP)
   * IXON: Control pausing and resuming the transmission Ctrl-S and Ctrl-Q.
   * IEXTEN: Disable Ctrl-V
   * ICRNL: Fix Ctrl-M
   * OPOST: Turn off all output.
   * BRKINT: a break condition will cause a SIGINT signal to be sent to the program
   * like pressing Ctrl-C.
   * INPCK: Enable parity checking.
   * ISTRIP: causes the 8th bit of each input byte to be stripped.
   * CS8: is not a flag. It sets the character size (CS) to 8 bits per byte.
   ****************************************************************************/
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}
