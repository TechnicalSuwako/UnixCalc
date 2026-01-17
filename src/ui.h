#pragma once

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#define FGCOL 0xfcfcfc
#if defined(__OpenBSD__)
#define BGCOL 0x232320
#define BTSEL 0xb8b515
#define BTCOL 0xf1ed25
#define BTHVR 0xecea71
#elif defined(__OpenBSD__)
#define BGCOL 0x232020
#define BTSEL 0xb61729
#define BTCOL 0xee4030
#define BTHVR 0xf35869
#endif

typedef struct {
  int x, y, width, height;
} SuwaWindow;

typedef struct {
  int x, y, width, height;
  const char *text;
  XftFont *font;
  int bg_color;
  XftColor fg_color;
  int pressed; // 0 = 普通、1 = 押している
} SuwaButton;

typedef struct {
  int x, y, width, height;
  char text[64];
  XftFont *font;
  XftColor fg_color;
} SuwaLabel;

typedef struct {
  int isrunning;
  Display *display;
  Window xwindow;
  Drawable target;
  GC gc;
  Visual visual;
  XftColor color;
  XftFont *font;
  Colormap colormap;
  Pixmap backbuf;
  SuwaWindow window;
  SuwaLabel resLabel;
  SuwaLabel problemLabel;
} UiSystem;
