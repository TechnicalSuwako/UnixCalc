#pragma once

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include "ui.h"

#define FGCOL 0xfcfcfc
#define BGCOL 0x232020
#define BTSEL 0xb61729
#define BTCOL 0xee4030
#define BTHVR 0xf35869

extern int window_width;
extern int window_height;
extern int isrunning;
extern char displayprb[128];
extern char curinput[256];
extern char displaytxt[64];
extern int input_pos;
extern Pixmap backbuf;
extern SuwaButton *hovered_btn;
extern SuwaButton *pressed_btn;
