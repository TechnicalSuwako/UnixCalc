#include "utils.h"

void cleanup(UiSystem *ui) {
  // フォント
  if (ui->resLabel.font) XftFontClose(ui->display, ui->resLabel.font);
  if (ui->problemLabel.font) XftFontClose(ui->display, ui->problemLabel.font);
  if (ui->font) XftFontClose(ui->display, ui->font);

  // その他
  if (ui->gc) XFreeGC(ui->display, ui->gc);
  if (ui->backbuf) {
    XFreePixmap(ui->display, ui->backbuf);
    ui->backbuf = None;
  }
  if (ui->xwindow) XDestroyWindow(ui->display, ui->xwindow);
  if (ui->display) XCloseDisplay(ui->display);
}
