#include "utils.h"

void cleanup(Display *display, Window window, GC gc,
    XftColor *color, XftColor *btncolor, XftFont *font,
    XftFont *disfont, XftFont *prbfont, Colormap colormap, Visual *visual,
    Pixmap backbuf) {
  if (btncolor) XftColorFree(display, visual, colormap, btncolor);
  if (color) XftColorFree(display, visual, colormap, color);
  if (disfont) XftFontClose(display, disfont);
  if (prbfont) XftFontClose(display, prbfont);
  if (font) XftFontClose(display, font);
  if (gc) XFreeGC(display, gc);
  if (backbuf) {
    XFreePixmap(display, backbuf);
    backbuf = None;
  }
  if (window) XDestroyWindow(display, window);
  if (display) XCloseDisplay(display);
}
