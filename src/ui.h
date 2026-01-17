#pragma once

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

typedef struct {
  Display *display;
  Window window;
  Drawable target;
  GC gc;
  Visual visual;
  XftDraw *xftdraw;
  XftColor color, btncolor, textcolor;
  XftFont *font, *prbfont, *disfont;
  Colormap colormap;
  Pixmap backbuf;
} UiSystem;

typedef struct {
  int x, y, width, height;
  const char *label;
  int pressed; // 0 = 普通、1 = 押している
} SuwaButton;
