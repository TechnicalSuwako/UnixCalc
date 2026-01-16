#include "display.h"
#include "program.h"

#include <string.h>

void drawbuttons(Display *dpy, Drawable target, GC gc, SuwaButton *btn,
    XftDraw *xftdraw, XftColor *textcolor, XftFont *font) {
  unsigned long curbg = btn->pressed ? BTSEL : BTCOL;
  XSetForeground(dpy, gc, curbg);
  XFillRectangle(dpy, target, gc, btn->x, btn->y, btn->width, btn->height);

  // 文字の中央に
  if (btn->label && font && xftdraw) {
    const FcChar8 *str = (const FcChar8 *)btn->label;
    int len = strlen((const char *)str);

    XGlyphInfo extents;
    XftTextExtentsUtf8(dpy, font, str, len, &extents);

    int text_w = extents.xOff;
    int text_h = font->ascent + font->descent;

    int tx = btn->x + (btn->width - text_w) / 2;
    int ty = btn->y + (btn->height - text_h) / 2 + font->ascent;

    // tx -= extents.x;

    XftDrawStringUtf8(xftdraw, textcolor, font, tx, ty, (FcChar8 *)btn->label, len);
  }
}
