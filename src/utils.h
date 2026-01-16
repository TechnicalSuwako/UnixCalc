#pragma once

#include "program.h"

void cleanup(Display *display, Window window, GC gc,
    XftColor *color, XftColor *btncolor, XftFont *font,
    XftFont *disfont, XftFont *prbfont, Colormap colormap, Visual *visual,
    Pixmap backbuf);
