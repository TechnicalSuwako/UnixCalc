#include "display.h"
#include "program.h"

#include <string.h>

void drawbuttons(UiSystem *ui, SuwaButton *btn, XftDraw *xftdraw) {
  unsigned long curbg = btn->pressed ? BTSEL : BTCOL;
  XSetForeground(ui->display, ui->gc, curbg);
  XFillRectangle(ui->display, ui->target, ui->gc, btn->x, btn->y,
      btn->width, btn->height);

  // 文字の中央に
  if (btn->label && ui->font && xftdraw) {
    const FcChar8 *str = (const FcChar8 *)btn->label;
    int len = strlen((const char *)str);

    XGlyphInfo extents;
    XftTextExtentsUtf8(ui->display, ui->font, str, len, &extents);

    int text_w = extents.xOff;
    int text_h = ui->font->ascent + ui->font->descent;

    int tx = btn->x + (btn->width - text_w) / 2;
    int ty = btn->y + (btn->height - text_h) / 2 + ui->font->ascent;

    // tx -= extents.x;

    XftDrawStringUtf8(xftdraw, &ui->color, ui->font, tx, ty,
        (FcChar8 *)btn->label, len);
  }
}
