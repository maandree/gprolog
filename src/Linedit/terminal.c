/*-------------------------------------------------------------------------*
 * GNU Prolog                                                              *
 *                                                                         *
 * Part  : line-edit library                                               *
 * File  : terminal.c                                                      *
 * Descr.: basic terminal operations                                       *
 * Author: Daniel Diaz                                                     *
 *                                                                         *
 * Copyright (C) 1999-2013 Daniel Diaz                                     *
 *                                                                         *
 * This file is part of GNU Prolog                                         *
 *                                                                         *
 * GNU Prolog is free software: you can redistribute it and/or             *
 * modify it under the terms of either:                                    *
 *                                                                         *
 *   - the GNU Lesser General Public License as published by the Free      *
 *     Software Foundation; either version 3 of the License, or (at your   *
 *     option) any later version.                                          *
 *                                                                         *
 * or                                                                      *
 *                                                                         *
 *   - the GNU General Public License as published by the Free             *
 *     Software Foundation; either version 2 of the License, or (at your   *
 *     option) any later version.                                          *
 *                                                                         *
 * or both in parallel, as here.                                           *
 *                                                                         *
 * GNU Prolog is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * General Public License for more details.                                *
 *                                                                         *
 * You should have received copies of the GNU General Public License and   *
 * the GNU Lesser General Public License along with this program.  If      *
 * not, see http://www.gnu.org/licenses/.                                  *
 *-------------------------------------------------------------------------*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#include "../EnginePl/gp_config.h"

#if defined(__unix__) || defined(__CYGWIN__)

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/uio.h>


#if defined(HAVE_SYS_IOCTL_COMPAT_H)
#include <sys/ioctl_compat.h>
#endif

#if defined(HAVE_TERMIOS_H)
#include <termios.h>
typedef struct termios TermIO;

#define Gtty(fd, s) tcgetattr(fd, s)
#define Stty(fd, s) tcsetattr(fd, TCSANOW, s)
#else
#include <termio.h>
typedef struct termio TermIO;

#define Gtty(fd, s) ioctl(fd, TCGETA, s)
#define Stty(fd, s) ioctl(fd, TCSETA, s)
#endif /* !HAVE_TERMIOS_H */

#elif defined(_WIN32)

#include <windows.h>
#include <io.h>
#include <conio.h>

#endif


#define LE_DEFINE_HOOK_MACROS

#define TERMINAL_FILE

#include "terminal.h"
#include "linedit.h"




/*---------------------------------*
 * Constants                       *
 *---------------------------------*/

/*---------------------------------*
 * Type Definitions                *
 *---------------------------------*/

/*---------------------------------*
 * Global Variables                *
 *---------------------------------*/

static int use_linedit;
static int use_gui;
static int use_ansi;
#if defined(__unix__) || defined(__CYGWIN__)
static int fd_in = 0;           /* not changed */
#endif
static int fd_out = -1;

#if defined(__unix__) || defined(__CYGWIN__)

static int is_tty_in;
static int is_tty_out;
static TermIO old_stty_in;
static TermIO new_stty_in;
static TermIO old_stty_out;
static TermIO new_stty_out;

static int nb_rows, nb_cols;
static int pos;

#elif defined(_WIN32)

static HANDLE h_stdin;
static HANDLE h_stdout;
static DWORD im;

static int code_page = 0;
static int oem_put = 1;
static int oem_get = 1;

#endif

static int interrupt_key;




/*---------------------------------*
 * Function Prototypes             *
 *---------------------------------*/

static void Parse_Env_Var(void);

#if defined(__unix__) || defined(__CYGWIN__)

static void Choose_Fd_Out(void);

static void Set_TTY_Mode(TermIO *old, TermIO *new);

#endif

static int LE_Get_Char0(void);

static void Backd(int n);

static void Forwd(int n, char *str);

static void Displ(int n, char *str);

static void Displ_Str(char *s);

static void Erase(int n);




/*-------------------------------------------------------------------------*
 * PL_LE_INITIALIZE                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/
int
Pl_LE_Initialize(void)
{
  static int initialized = 0;
  static int le_mode;

  if (initialized)
    return le_mode;

  initialized = 1;
  
  Parse_Env_Var();

  if (!use_linedit)
    return (le_mode = LE_MODE_DEACTIVATED);

   le_mode = LE_MODE_TTY;	/* default */


#if defined(__unix__) || defined(__CYGWIN__)
  Choose_Fd_Out();
#endif

  if (pl_le_hook_start && use_gui)
    (*pl_le_hook_start) (use_gui == 2);

  if (pl_le_hook_put_char != NULL && pl_le_hook_get_char0 != NULL
      && pl_le_hook_kbd_is_not_empty != NULL && pl_le_hook_screen_size != NULL)
    le_mode = LE_MODE_HOOK;
  else
    {
      pl_le_hook_put_char = NULL;
      pl_le_hook_get_char0 = NULL;
      pl_le_hook_kbd_is_not_empty = NULL;
      pl_le_hook_screen_size = NULL;
    }

#define INIT_FCT(hook, def) if (hook == NULL) hook = def

  /* inside terminal.c */
  INIT_FCT(pl_le_hook_screen_size, Pl_LE_Screen_Size);
  INIT_FCT(pl_le_hook_kbd_is_not_empty, Pl_LE_Kbd_Is_Not_Empty);
  INIT_FCT(pl_le_hook_put_char, Pl_LE_Put_Char);
  INIT_FCT(pl_le_hook_get_char0, LE_Get_Char0);
  INIT_FCT(pl_le_hook_ins_mode, Pl_LE_Ins_Mode);
  INIT_FCT(pl_le_hook_emit_beep, Pl_LE_Emit_Beep);

  /* inside linedit.c */
  INIT_FCT(pl_le_hook_backd, Backd);
  INIT_FCT(pl_le_hook_forwd, Forwd);
  INIT_FCT(pl_le_hook_displ, Displ);
  INIT_FCT(pl_le_hook_erase, Erase);
  INIT_FCT(pl_le_hook_displ_str, Displ_Str);


#if defined(__unix__) || defined(__CYGWIN__)

#elif defined(_WIN32)

  if (pl_le_hook_put_char == Pl_LE_Put_Char)    /* DOS console mode */
    {
      h_stdin = GetStdHandle(STD_INPUT_HANDLE);
      h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    }

  interrupt_key = KEY_CTRL('C');        /* WIN32: interrupt = CTRL+C */

#endif

  return le_mode;
}




/*-------------------------------------------------------------------------*
 * PARSE_ENV_VAR                                                           *
 *                                                                         *
 *-------------------------------------------------------------------------*/
static void
Parse_Env_Var(void)
{
  char *p;
  char buff[1024];
  char *q;

  use_linedit = use_gui = use_ansi = 1; /* default */

  p = getenv("LINEDIT");
  if (p == NULL)
    return;

  if (strncmp(p, "no", 2) == 0)	/* deactivate linedit */
    {
      use_linedit = 0;
      return;
    }

  if (strstr(p, "gui=n") != NULL)
    use_gui = 0;

  if (strstr(p, "gui=s") != NULL) /* silent */
    use_gui = 2;

  if (strstr(p, "ansi=n") != NULL)
    use_ansi = 0;

#ifdef _WIN32
  if ((q = strstr(p, "cp=")) != NULL && isdigit(q[3]))
    code_page = strtol(q + 3, NULL, 10), printf("cp read:%d\n", code_page);

  if (strstr(p, "oem_put=n") != NULL)
    oem_put = 0;

  if (strstr(p, "oem_put=y") != NULL)
    oem_put = 1;

  if (strstr(p, "oem_get=n") != NULL)
    oem_get = 0;

  if (strstr(p, "oem_get=y") != NULL)
    oem_get = 1;
#endif

  if ((p = strstr(p, "out=")) != NULL)
    {
      p += 4;

      if (isdigit(*p))
        fd_out = strtol(p, NULL, 10);
      else
        {
          while(*p && isprint(*p) && !isspace(*p))
            *q++ = *p++;

          *q = '\0';
          fd_out = open(buff, O_WRONLY); /* on error fd_out = -1 */
        }
    }

  return;
}




#if defined(__unix__) || defined(__CYGWIN__)

/*-------------------------------------------------------------------------*
 * CHOOSE_FD_OUT                                                           *
 *                                                                         *
 *-------------------------------------------------------------------------*/
static void
Choose_Fd_Out(void)
{
  int fd[3] = { 1, 0, 2 };      /* order fd list to try to find a tty */
  int i, try;
  int mask;
  char *p;

  for(i = 0; i < 3 && fd_out < 0; i++)
    {
      try = fd[i];

      if (!isatty(try))
        continue;

      mask = fcntl(try, F_GETFL);
      if ((mask & O_WRONLY) == O_WRONLY || (mask & O_RDWR) == O_RDWR)
        {
          fd_out = try;
          break;
        }

      if ((p = ttyname(try)) != NULL)
        fd_out = open(p, O_WRONLY);
    }

  if (fd_out < 0)
    fd_out = 1;

}
#endif


/*-------------------------------------------------------------------------*
 * PL_LE_OPEN_TERMINAL                                                     *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Pl_LE_Open_Terminal(void)
{
  fflush(stdout);
  fflush(stderr);


#if defined(__unix__) || defined(__CYGWIN__) /* Mode cbreak (raw mode) */
  is_tty_in = !Gtty(fd_in, &old_stty_in);
  is_tty_out = !Gtty(fd_out, &old_stty_out);

  if (is_tty_in)
    {
      interrupt_key = old_stty_in.c_cc[VINTR];
      Set_TTY_Mode(&old_stty_in, &new_stty_in);
      Stty(fd_in, &new_stty_in);
    }
  else
    interrupt_key = KEY_CTRL('C');

  if (is_tty_out)
    {
      Set_TTY_Mode(&old_stty_out, &new_stty_out);
      Stty(fd_out, &new_stty_out);
    }

  Pl_LE_Screen_Size(&nb_rows, &nb_cols);

  pos = 0;

#elif defined(_WIN32)

  if (pl_le_hook_put_char == Pl_LE_Put_Char)    /* DOS console mode */
    {
      h_stdin = GetStdHandle(STD_INPUT_HANDLE);
      h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
      GetConsoleMode(h_stdin, &im);
      SetConsoleMode(h_stdin, im & ~ENABLE_PROCESSED_INPUT);
      if (code_page && (!SetConsoleCP(code_page) || !SetConsoleOutputCP(code_page)))
	printf("warning: Setting console code page to %d failed (error: %d)\n", code_page, (int) GetLastError());
    }

  interrupt_key = KEY_CTRL('C');        /* WIN32: interrupt = CTRL+C */

#endif
}




/*-------------------------------------------------------------------------*
 * PL_LE_CLOSE_TERMINAL                                                    *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Pl_LE_Close_Terminal(void)
{
#if defined(__unix__) || defined(__CYGWIN__) /* Initial mode (cooked mode) */

  if (is_tty_in)
    Stty(fd_in, &old_stty_in);

  if (is_tty_out)
    Stty(fd_out, &old_stty_out);

#elif defined(_WIN32)

  if (pl_le_hook_put_char == Pl_LE_Put_Char)    /* DOS console mode */
    SetConsoleMode(h_stdin, im);

#endif
}




#if defined(__unix__) || defined(__CYGWIN__)

/*-------------------------------------------------------------------------*
 * SET_TTY_MODE                                                            *
 *                                                                         *
 * Mode cbreak (raw mode).                                                 *
 *-------------------------------------------------------------------------*/
static void
Set_TTY_Mode(TermIO *old, TermIO *new)
{
  *new = *old;

  new->c_iflag &= ~(INLCR | IGNCR | ICRNL | IXON | IXOFF);
  new->c_oflag = OPOST | ONLCR;
  new->c_lflag &= ~(ICANON | ECHO | ECHONL);

  new->c_cc[VMIN] = 1;          /* MIN # of chars */
  new->c_cc[VTIME] = 1;         /* TIME */

  new->c_cc[VINTR] = -1;        /* deactivate SIGINT signal */
}

#endif




/*-------------------------------------------------------------------------*
 * PL_LE_SCREEN_SIZE                                                       *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Pl_LE_Screen_Size(int *row, int *col)
{
#if defined(__unix__) || defined(__CYGWIN__)
  struct winsize ws;

  if (!is_tty_out)
    {
      row = col = 0;
      return;
    }

  ioctl(fd_out, TIOCGWINSZ, &ws);
  nb_rows = *row = ws.ws_row;
  nb_cols = *col = ws.ws_col;

#elif defined(_WIN32)

  CONSOLE_SCREEN_BUFFER_INFO csbi;

  if (GetConsoleScreenBufferInfo(h_stdout, &csbi))
    {
      *row = csbi.dwSize.Y;
      *col = csbi.dwSize.X;
    }
  else
    {
      *row = 25;
      *col = 80;
    }

#endif
}




/*-------------------------------------------------------------------------*
 * PL_LE_IS_INTERRUPT_KEY                                                  *
 *                                                                         *
 *-------------------------------------------------------------------------*/
int
Pl_LE_Is_Interrupt_Key(int c)
{
  return (c == interrupt_key);
}




/*-------------------------------------------------------------------------*
 * PL_LE_KBD_IS_NOT_EMPTY                                                  *
 *                                                                         *
 *-------------------------------------------------------------------------*/
int
Pl_LE_Kbd_Is_Not_Empty(void)
{
#if defined(__unix__) || defined(__CYGWIN__)

#ifdef FIONREAD
  int nb_not_read;

  ioctl(fd_in, FIONREAD, &nb_not_read);
  return nb_not_read != 0;
#else
  return 0;
#endif

#elif defined(_WIN32)

  return kbhit();

#endif
}




/*-------------------------------------------------------------------------*
 * PL_LE_INS_MODE                                                          *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Pl_LE_Ins_Mode(int ins_mode)
{
#if defined(_WIN32) && !defined(__CYGWIN__)

  CONSOLE_CURSOR_INFO cci;

  if (!GetConsoleCursorInfo(h_stdout, &cci))
    return;

  cci.dwSize = (ins_mode) ? 5 : 50;

  SetConsoleCursorInfo(h_stdout, &cci);

#endif
}




/*-------------------------------------------------------------------------*
 * PL_LE_EMIT_BEEP                                                         *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Pl_LE_Emit_Beep(void)
{
#if defined(__unix__) || defined(__CYGWIN__)

  Pl_LE_Put_Char('\a');

#else

  Beep(800, 220);

#endif
}




/*
 * Character I/O
 */



/*-------------------------------------------------------------------------*
 * PL_LE_PUT_CHAR                                                          *
 *                                                                         *
 *-------------------------------------------------------------------------*/
void
Pl_LE_Put_Char(int c)
{
#if defined(__unix__) || defined(__CYGWIN__)
  char c0 = c;

  if (use_ansi)
    {
      char buf[20];

      switch(c)
        {
        case '\b':
          if (pos == 0)
            {
              pos = nb_cols - 1;
              sprintf(buf, "\033[A\033[%dC", pos);
              if (write(fd_out, buf, strlen(buf))) /* to avoid gcc warning warn_unused_result */
		{
		}


              return;
            }
          pos--;
          break;

        case '\a':
          break;

        case '\n':
          pos = 0;
          break;

        default:
          if (++pos > nb_cols)
            pos = 1;
        }
    }
 
  c0 = c;
  if (write(fd_out, &c0, 1))  /* to avoid gcc warning warn_unused_result */
    {
    }

#elif defined(_WIN32)

  CONSOLE_SCREEN_BUFFER_INFO csbi;

  if (c != '\b')
    {
      if (oem_put)
	{
	  char buff[2];

	  buff[0] = c;
	  buff[1] = '\0';
	  CharToOem(buff, buff);
	  c = buff[0];
	}

#if 0
      putch(c);
#else  /* replacement of putch() same but a bit faster */
      {
	DWORD nb;
	char c0 = (char) c;

	WriteConsole(h_stdout, &c0, 1, &nb, NULL);
      }
#endif
      return;
    }

  GetConsoleScreenBufferInfo(h_stdout, &csbi);

  if (csbi.dwCursorPosition.X == 0)
    {
      csbi.dwCursorPosition.X = csbi.dwSize.X - 1;
      csbi.dwCursorPosition.Y--;
    }
  else
    csbi.dwCursorPosition.X--;

  SetConsoleCursorPosition(h_stdout, csbi.dwCursorPosition);

#endif
}




/*-------------------------------------------------------------------------*
 * PL_LE_GET_CHAR                                                          *
 *                                                                         *
 *-------------------------------------------------------------------------*/
int
Pl_LE_Get_Char(void)
{
  int c;

  c = GET_CHAR0;

#ifdef USE_UTF8
  if (c & 0x80) /* More bytes needed for a complete character */
    {
      int n = 0;

      while (c & 0x80) /* The first byte leads with as many zeroes as there are bytes */
        {
          c <<= 1;
          ++n;
        }
      c = (c & 0xFF) >> n;

      while (--n)
        {
          int ucs_c;

          ucs_c = GET_CHAR0;
          c = (c << 6) | (ucs_c & 0x3F);
            /* All following bytes have c the two highest bits sets to true and false, respectively.
             * All bytes that do no have only meaning in how to parse are character bits in the order
             * the occur, high bits high, early bytes even higher. */
        }
    }
#endif

  if (c == 0x1b)
    {
      int esc_c;

      esc_c = GET_CHAR0;
#if defined(__unix__) || defined(__CYGWIN__)
      if (esc_c == '[') /* keyboard ANSI CSI escape sequence */
        {
          c = 0;
          esc_c = GET_CHAR0;
          while ((esc_c == ';') || (('0' <= esc_c) && (esc_c <= '9')))
            {
              c = c * 11 + (esc_c == ';' ? 10 : esc_c - '0');
              esc_c = GET_CHAR0;
            }
          c = (c << 8) | (1 << 31) | esc_c;
	  /* 1 << 31 is required so it does not interfere with UTF-8 decoding.
	   * UCS, the characters defined by Unicode and are coverted by UTF-8
	   * are limited to 31 bits, not 32, meaning that the 31:th bit is never
	   * set (only non-negative characters exists) and negative characters
	   * can be used internally the the program. */
        }
      else if (esc_c == 'O') /* keyboard ANSI ESC O escape sequence */
        {
          c = GET_CHAR0;
          switch (c)
            {
            case 'P':  c = KEY_EXT_FCT_1;  break;
            case 'Q':  c = KEY_EXT_FCT_2;  break;
            case 'R':  c = KEY_EXT_FCT_3;  break;
            case 'S':  c = KEY_EXT_FCT_4;  break;
            case 'H':  c = KEY_EXT_HOME;   break;
            case 'F':  c = KEY_EXT_END;    break;
            }
        }
      else
#endif
        c = KEY_ESC(esc_c);
    }

  return c;
}



#ifdef USE_UTF8
/*-------------------------------------------------------------------------*
 * LE_GET_ENCODE_UTF8                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/
char *
Pl_LE_Encode_UTF8(int *utf32)
{
  int ptr, size;
  char *utf8;
  char buff[8];

  ptr = 0;
  size = 64;
  utf8 = malloc(64 * sizeof(char));

  while (*utf32)
    {
      int c;

      c = *utf32++;

      if (c <= 0x7F) /* If the character finns in ASCII, it is ASCII encoded */
        {
          if (ptr == size)
            utf8 = realloc(utf8, (size <<= 1) * sizeof(char));
          *(utf8 + ptr++) = (char) c;
        }
      else
        {
          int n;
          int mask;

          n = 0;

          while (c) /* Split the character by its bits */
            {
              buff[8 - ++n] = (char) (c & 0x3F);
              c >>= 6;
            }

          mask = ((2 << n) - 1) << (7 - n); /* Mask should cover n 1:s and one 0 as the high bytes */

          if (mask & c) /* The number if byte bits does not fit*/
            buff[8 - ++n] = 0;

          buff[8 - n] |= (char) ((mask << 1) & 0xFF); /* Remove the bit covering the zero*/

          /* And now to add the bytes in the correct order, and set the highet bit for all bytes*/
          if (ptr + n == size)
            utf8 = realloc(utf8, (size <<= 1) * sizeof(char));
          for (n = 8 - n; n < 8; n++)
            *(utf8 + ptr++) = (char) (buff[n] | 0x80);
        }
    }

  /* Add the NUL termination */
  if (ptr == size)
    utf8 = realloc(utf8, (size + 1) * sizeof(char));
  *(utf8 + ptr) = 0;

  return utf8;
}
#endif




/*-------------------------------------------------------------------------*
 * LE_GET_CHAR0                                                            *
 *                                                                         *
 *-------------------------------------------------------------------------*/
static int
LE_Get_Char0(void)
{
#if defined(__unix__) || defined(__CYGWIN__)
  unsigned char c;

  if (read(fd_in, &c, 1) != 1)
    return KEY_CTRL('D');
  return (int) c;

#elif defined(_WIN32)

  INPUT_RECORD ir;
  DWORD nb;
  int c;

 read_char:
  if (!ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &nb) || nb != 1)
    return -1;

  switch (ir.EventType)
    {
    case KEY_EVENT:
      if (!ir.Event.KeyEvent.bKeyDown)
        goto read_char;
      c = ir.Event.KeyEvent.uChar.AsciiChar & 0xff;
      if (c == 0 || c == 0xe0)
        {
          c = ir.Event.KeyEvent.wVirtualKeyCode;
          if (c < 0x15 || c > 0x87)     /* e.g. CTRL key alone */
            goto read_char;
          if (ir.Event.KeyEvent.dwControlKeyState &
              (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED))
            c = (2 << 8) | c;
          else
            c = (1 << 8) | c;
        }
      else if (oem_get)
        {
	  char buff[2];
	  
          buff[0] = c;
          buff[1] = '\0';
          OemToChar(buff, buff);
          c = buff[0];
        }
      break;

    case MOUSE_EVENT:
    case WINDOW_BUFFER_SIZE_EVENT:
    case MENU_EVENT:
    case FOCUS_EVENT:
      goto read_char;
      break;
      }

  return c;

#endif
}




/*-------------------------------------------------------------------------*
 * BACKD                                                                   *
 *                                                                         *
 *-------------------------------------------------------------------------*/
static void
Backd(int n)
{
  while (n--)
    PUT_CHAR('\b');
}




/*-------------------------------------------------------------------------*
 * FORWD                                                                   *
 *                                                                         *
 *-------------------------------------------------------------------------*/
static void
Forwd(int n, char *str)
{
  while (n--)
    PUT_CHAR(*str++);
}




/*-------------------------------------------------------------------------*
 * DISPL                                                                   *
 *                                                                         *
 *-------------------------------------------------------------------------*/
static void
Displ(int n, char *str)
{
  while (n--)
    PUT_CHAR(*str++);
}




/*-------------------------------------------------------------------------*
 * ERASE                                                                   *
 *                                                                         *
 *-------------------------------------------------------------------------*/
static void
Erase(int n)
{
  int n0 = n;

  while (n--)
    PUT_CHAR(' ');

  BACKD(n0);
}




/*-------------------------------------------------------------------------*
 * DISPL_STR                                                               *
 *                                                                         *
 *-------------------------------------------------------------------------*/
static void
Displ_Str(char *str)
{
  while (*str)
    PUT_CHAR(*str++);
}
