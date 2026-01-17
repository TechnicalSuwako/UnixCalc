#include "utils.h"

void cleanup(UiSystem *ui) {
  if (ui->textcolor.pixel != 0)
    XftColorFree(ui->display, &ui->visual, ui->colormap, &ui->textcolor);
  if (ui->btncolor.pixel != 0)
    XftColorFree(ui->display, &ui->visual, ui->colormap, &ui->btncolor);
  if (ui->color.pixel != 0)
    XftColorFree(ui->display, &ui->visual, ui->colormap, &ui->color);
  if (ui->disfont) XftFontClose(ui->display, ui->disfont);
  if (ui->prbfont) XftFontClose(ui->display, ui->prbfont);
  if (ui->font) XftFontClose(ui->display, ui->font);
  if (ui->gc) XFreeGC(ui->display, ui->gc);
  if (ui->backbuf) {
    XFreePixmap(ui->display, ui->backbuf);
    ui->backbuf = None;
  }
  if (ui->window) XDestroyWindow(ui->display, ui->window);
  if (ui->display) XCloseDisplay(ui->display);
}
