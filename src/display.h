#pragma once

#include "program.h"

void drawbuttons(Display *dpy, Drawable target, GC gc, SuwaButton *btn,
    XftDraw *xftdraw, XftColor *textcolor, XftFont *font);
