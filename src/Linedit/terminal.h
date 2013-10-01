/*-------------------------------------------------------------------------*
 * GNU Prolog                                                              *
 *                                                                         *
 * Part  : line-edit library                                               *
 * File  : terminal.h                                                      *
 * Descr.: basic terminal operations - header file                         *
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


/*---------------------------------*
 * Constants                       *
 *---------------------------------*/


#define KEY_CTRL(x)                ((x) & 0x1f)
#define KEY_ALT(x)                 ((255 << (32 - 8)) | (x) | 0x20)

#define KEY_MODIF_NONE             0
#define KEY_MODIF_SHIFT            1
#define KEY_MODIF_ALT              2
#define KEY_MODIF_CTRL             4

#define ESC                        0x1b
#define KEY_BACKSPACE              '\b'
#define KEY_DELETE                 0x7f


#if defined(__unix__) || defined(__CYGWIN__)	/* Unix */

/* Macros used to defined key stroke value constants */
#define _k(a, b)                   ((a) * 11 + (b))
#define _K(a, c)                   (((a) << 7) | (1 << 31) | (c))
#define _K0(a)                     _K(0, a)
#define _K1(a, b)                  _K(a, b)
#define _K2(a, b, c)               _K(_k(a, b), c)
#define _K3(a, b, c, d)            _K(_k(_k(a, b), c), d)
#define _K4(a, b, c, d, e)         _K(_k(_k(_k(a, b), c), d), e)
#define _KX(x, k)                  ((k) | ((x) << (32 - 8)))

#define _F(n, op)                  ((11 + (n - 1) * 6 / 5) op 10)
#define KEY_EXT_FCT(n, mod)        (!mod ? _K2(_F(n, /), _F(n, %), '~') : _K4(_F(n, /), _F(n, %), 10, mod + 1, '~'))

#define KEY_EXT_FCT_1(mod)         KEY_EXT_FCT(1, mod)  /* ESC 1 1 ~ */
#define KEY_EXT_FCT_2(mod)         KEY_EXT_FCT(2, mod)  /* ESC 1 2 ~ */
#define KEY_EXT_FCT_3(mod)         KEY_EXT_FCT(3, mod)  /* ESC 1 3 ~ */
#define KEY_EXT_FCT_4(mod)         KEY_EXT_FCT(4, mod)  /* ESC 1 4 ~ */
#define KEY_EXT_FCT_5(mod)         KEY_EXT_FCT(5, mod)  /* ESC 1 5 ~ */

#define KEY_EXT_FCT_6(mod)         KEY_EXT_FCT(6, mod)  /* ESC 1 7 ~ */
#define KEY_EXT_FCT_7(mod)         KEY_EXT_FCT(7, mod)  /* ESC 1 8 ~ */
#define KEY_EXT_FCT_8(mod)         KEY_EXT_FCT(8, mod)  /* ESC 1 9 ~ */
#define KEY_EXT_FCT_9(mod)         KEY_EXT_FCT(9, mod)  /* ESC 2 0 ~ */
#define KEY_EXT_FCT_10(mod)        KEY_EXT_FCT(10, mod) /* ESC 2 1 ~ */

#define KEY_EXT_FCT_11(mod)        KEY_EXT_FCT(11, mod) /* ESC 2 3 ~ */
#define KEY_EXT_FCT_12(mod)        KEY_EXT_FCT(12, mod) /* ESC 2 4 ~ */

#define KEY_EXT_ARROW(c, mod)      (!mod ? _K0(c)  : _K3(1, 10, mod + 1, c))
#define KEY_EXT_UP(mod)            KEY_EXT_ARROW('A', mod)
#define KEY_EXT_DOWN(mod)          KEY_EXT_ARROW('B', mod)
#define KEY_EXT_RIGHT(mod)         KEY_EXT_ARROW('C', mod)
#define KEY_EXT_LEFT(mod)          KEY_EXT_ARROW('D', mod)

#define KEY_EXT_(n, mod)           (!mod ? _K1(n, '~') : _K3(n, 10, mod + 1, '~'))
#define KEY_EXT_HOME(mod)          KEY_EXT_(1, mod)
#define KEY_EXT_END(mod)           KEY_EXT_(4, mod)
#define KEY_EXT_PAGE_UP(mod)       KEY_EXT_(5, mod)
#define KEY_EXT_PAGE_DOWN(mod)     KEY_EXT_(6, mod)
#define KEY_EXT_INSERT(mod)        KEY_EXT_(2, mod)
#define KEY_EXT_DELETE(mod)        KEY_EXT_(3, mod)

#define KEY_EXT_FCT_1_ALT          _KX('O', _K0('P'))
#define KEY_EXT_FCT_2_ALT          _KX('O', _K0('Q'))
#define KEY_EXT_FCT_3_ALT          _KX('O', _K0('R'))
#define KEY_EXT_FCT_4_ALT          _KX('O', _K0('S'))
#define KEY_EXT_HOME_ALT           _KX('O', _K0('H'))
#define KEY_EXT_END_ALT            _KX('O', _K0('F'))
#define CYGWIN_FCT(n)              _KX('[', _K0('@' + (n)))

#define _unnil(value)              (value ? value : 1)
#define _comb(a, b)                ((a) * 121 + 110 + (b))
#define _cygwin_mod(c)             ((c == '^' ? KEY_MODIF_CTRL : c == '$' ? KEY_MODIF_SHIFT : 0) + 1)
#define WITH_MODIF(value, end)     (((end == '^') || (end == '$')) ? _comb(_unnil(value), _cygwin_mod(end)) : value)
#define WITHOUT_MODIF(end)         (((end == '^') || (end == '$')) ? '~' : end)

#elif defined(_WIN32)		/* Win32 */

#include <windows.h>

#define MAKE_KEY(key, mod)         ((1 << 31) | ((mod) << (32 - 4)) | (key))

#define KEY_EXT_FCT_1(mod)         MAKE_KEY(VK_F1, mod)
#define KEY_EXT_FCT_2(mod)         MAKE_KEY(VK_F2, mod)
#define KEY_EXT_FCT_3(mod)         MAKE_KEY(VK_F3, mod)
#define KEY_EXT_FCT_4(mod)         MAKE_KEY(VK_F4, mod)
#define KEY_EXT_FCT_5(mod)         MAKE_KEY(VK_F5, mod)

#define KEY_EXT_FCT_6(mod)         MAKE_KEY(VK_F6, mod)
#define KEY_EXT_FCT_7(mod)         MAKE_KEY(VK_F7, mod)
#define KEY_EXT_FCT_8(mod)         MAKE_KEY(VK_F8, mod)
#define KEY_EXT_FCT_9(mod)         MAKE_KEY(VK_F9, mod)
#define KEY_EXT_FCT_10(mod)        MAKE_KEY(VK_F10, mod)

#define KEY_EXT_FCT_11(mod)        MAKE_KEY(VK_F11, mod)
#define KEY_EXT_FCT_12(mod)        MAKE_KEY(VK_F12, mod)

#define KEY_EXT_UP(mod)            MAKE_KEY(VK_UP, mod)
#define KEY_EXT_DOWN(mod)          MAKE_KEY(VK_DOWN, mod)
#define KEY_EXT_RIGHT(mod)         MAKE_KEY(VK_RIGHT, mod)
#define KEY_EXT_LEFT(mod)          MAKE_KEY(VK_LEFT, mod)

#define KEY_EXT_HOME(mod)          MAKE_KEY(VK_HOME, mod)
#define KEY_EXT_END(mod)           MAKE_KEY(VK_END, mod)
#define KEY_EXT_PAGE_UP(mod)       MAKE_KEY(VK_PRIOR, mod)
#define KEY_EXT_PAGE_DOWN(mod)     MAKE_KEY(VK_NEXT, mod)
#define KEY_EXT_INSERT(mod)        MAKE_KEY(VK_INSERT, mod)
#define KEY_EXT_DELETE(mod)        MAKE_KEY(VK_DELETE, mod)

#endif


#if defined(_WIN32)
#define KEY_IS_EOF(c)   ((c) == KEY_CTRL('D') || (c) == KEY_CTRL('Z'))
#else
#define KEY_IS_EOF(c)   ((c) == KEY_CTRL('D'))
#endif




/*---------------------------------*
 * Type Definitions                *
 *---------------------------------*/

/*---------------------------------*
 * Global Variables                *
 *---------------------------------*/

/*---------------------------------*
 * Function Prototypes             *
 *---------------------------------*/

void Pl_LE_Open_Terminal(void);

void Pl_LE_Close_Terminal(void);

void Pl_LE_Screen_Size(int *row, int *col);

void Pl_LE_Ins_Mode(int ins_mode);

int Pl_LE_Kbd_Is_Not_Empty(void);

int Pl_LE_Is_Interrupt_Key(int c);

void Pl_LE_Emit_Beep(void);

void Pl_LE_Put_Char(int c);

int Pl_LE_Get_Char(void);



