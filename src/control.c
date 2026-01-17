#include <X11/keysym.h>
#include <math.h>
#include <ctype.h>

#include "control.h"
#include "display.h"
#include "program.h"

static const char *btn_labels[][10] = {
  { NULL }, // 数字表示
  { "C", "±", "%", "÷" },
  { "7", "8", "9", "×" },
  { "4", "5", "6", "-" },
  { "1", "2", "3", "+" },
  { "0", ".", "=", "<" },
};

#define NUM_ROWS 6
#define NUM_COLS 6

static SuwaButton *find_button_at(int mx, int my) {
  static SuwaButton found = {0};

  int btn_w = 93;
  int btn_h = 60;
  int padding = 2;

  int row = -1;
  int col = -1;

  for (int r = 1; r < NUM_ROWS; ++r) {
    int y_start = 224 + (r - 1) * btn_h + padding;
    if (my >= y_start && my < y_start + btn_h) {
      row = r;
      break;
    }
  }

  if (row < 0) return NULL;

  for (int c = 0; c < NUM_COLS; ++c) {
    int x_start = padding + c * (btn_w + padding);
    if (mx >= x_start && mx < x_start + btn_w) {
      col = c;
      break;
    }
  }

  if (col < 0) return NULL;

  found.x = padding + col * (btn_w + padding);
  found.y = 224 + (row - 1) * (btn_h + padding);
  found.width = btn_w;
  found.height = btn_h;
  found.label = btn_labels[row][col];
  found.pressed = 1;

  return &found;
}

void append_to_input(char c) {
  if ((unsigned long)input_pos >= sizeof(curinput) - 2) return;
  strcpy(displayprb, "");
  curinput[input_pos++] = c;
  curinput[input_pos] = '\0';

  strncpy(displaytxt, curinput, sizeof(displaytxt) - 1);
  displaytxt[sizeof(displaytxt) - 1] = '\0';
}

void clear_calculator(void) {
  strcpy(displayprb, "");
  curinput[0] = '\0';
  input_pos = 0;
  strcpy(displaytxt, "0");
}

double evaluate_simple(const char *expr) {
  double res = 0.0;
  double cur = 0.0;
  char op = '+';
  int i = 0;
  snprintf(displayprb, sizeof(displayprb), "%s=", expr);

  while (expr[i]) {
    if (expr[i] == ' ') { i++; continue; }

    if (isdigit(expr[i]) || expr[i] == '.') {
      char numbuf[32];
      int j = 0;
      while (isdigit(expr[i]) || expr[i] == '.') {
        if (j < 31) numbuf[j++] = expr[i];
        ++i;
      }
      numbuf[j] = '\0';
      cur = atof(numbuf);
    } else {
      if (op == '+') res += cur;
      else if (op == '-') res -= cur;
      else if (op == '*') res *= cur;
      else if (op == '/') {
        if (cur == 0) return NAN;
        res /= cur;
      }

      op = expr[i];
      ++i;
      cur = 0.0;
    }
  }

  if (op == '+') res += cur;
  else if (op == '-') res -= cur;
  else if (op == '*') res *= cur;
  else if (op == '/') {
    if (cur == 0) return NAN;
    res /= cur;
  }

  return res;
}

void control_expose(UiSystem *ui, XEvent *e) {
  if (e->type != Expose && e->type != ConfigureNotify) return;

  XWindowAttributes attr;
  XGetWindowAttributes(ui->display, ui->window, &attr);
  int w = attr.width;
  int h = attr.height;

  if (ui->backbuf != None) {
    XFreePixmap(ui->display, ui->backbuf);
    ui->backbuf = None;
  }

  ui->backbuf = XCreatePixmap(ui->display, ui->window, w, h,
      DefaultDepth(ui->display, DefaultScreen(ui->display)));

  if (ui->backbuf == None) {
    fprintf(stderr, "バックバッファ作成失敗！\n");
    ui->backbuf = ui->window;
  }

  ui->target = ui->backbuf;

  XftDraw *backdraw = XftDrawCreate(ui->display, ui->backbuf,
      DefaultVisual(ui->display, DefaultScreen(ui->display)),
        DefaultColormap(ui->display, DefaultScreen(ui->display)));
  if (!backdraw) {
    fprintf(stderr, "Pixmap向けXftDrawの作成に失敗。\n");
    XFreePixmap(ui->display, ui->backbuf);
    ui->backbuf = None;
    return;
  }

  XSetForeground(ui->display, ui->gc, BGCOL);
  XFillRectangle(ui->display, ui->backbuf, ui->gc, 0, 0, w, h);

  // 出力
  if (displaytxt[0] != '\0' && ui->disfont) {
    const FcChar8 *text = (const FcChar8 *)displaytxt;
    int len = strlen((const char *)text);

    XGlyphInfo extents;
    XftTextExtentsUtf8(ui->display, ui->disfont, text, len, &extents);

    int tx = w - 20 - extents.xOff;
    int ty = 160;

    XftColor discol;
    XftColorAllocName(ui->display, DefaultVisual(ui->display, DefaultScreen(ui->display)),
                      DefaultColormap(ui->display, DefaultScreen(ui->display)),
                      "#ee4030", &discol);

    XftDrawStringUtf8(backdraw, &discol, ui->disfont, tx, ty, text, len);

    XftColorFree(ui->display, DefaultVisual(ui->display, DefaultScreen(ui->display)),
                 DefaultColormap(ui->display, DefaultScreen(ui->display)), &discol);
  }

  if (displayprb[0] != '\0' && ui->prbfont) {
    const FcChar8 *text = (const FcChar8 *)displayprb;
    int len = strlen((const char *)text);

    XGlyphInfo extents;
    XftTextExtentsUtf8(ui->display, ui->prbfont, text, len, &extents);

    int tx = w - 20 - extents.xOff;
    int ty = 80;

    XftColor discol;
    XftColorAllocName(ui->display,
        DefaultVisual(ui->display, DefaultScreen(ui->display)),
        DefaultColormap(ui->display, DefaultScreen(ui->display)),
        "#b61729", &discol);

    XftDrawStringUtf8(backdraw, &discol, ui->prbfont, tx, ty, text, len);

    XftColorFree(ui->display,
        DefaultVisual(ui->display, DefaultScreen(ui->display)),
        DefaultColormap(ui->display, DefaultScreen(ui->display)), &discol);
  }

  int width = 93;
  int height = 60;
  int padding = 2;
  printf("ウィンドウ: (%dx%d)\n", attr.width, attr.height);

  for (int row = 0; row < 6; ++row) {
    int y = 162 + row * (height + padding);
    if (y + height > h) break;

    int cols = 4;

    for (int col = 0; col < cols; ++col) {
      const char *label = btn_labels[row][col];
      if (!label) continue;

      int x = padding + col * (width + padding);

      SuwaButton btn;
      btn.x = x;
      btn.y = y;
      btn.width = width;
      btn.height = height;
      btn.label = label;
      btn.pressed = 0;

      drawbuttons(ui, &btn, backdraw);
    }
  }

  XCopyArea(ui->display, ui->backbuf, ui->window, ui->gc, 0, 0, w, h, 0, 0);
  XftDrawDestroy(backdraw);
  XFlush(ui->display);
}

void handle_button_press(UiSystem *ui, int mx, int my) {
  SuwaButton *btn = find_button_at(mx, my);
  if (!btn) return;

  btn->pressed = 1;
  XftDraw *backdraw = XftDrawCreate(ui->display, ui->backbuf,
      DefaultVisual(ui->display, DefaultScreen(ui->display)),
        DefaultColormap(ui->display, DefaultScreen(ui->display)));
  if (!backdraw) {
    fprintf(stderr, "Pixmap向けXftDrawの作成に失敗。\n");
    XFreePixmap(ui->display, ui->backbuf);
    ui->backbuf = None;
    return;
  }

  drawbuttons(ui, btn, backdraw);
  XFlush(ui->display);
}

void handle_button_release(UiSystem *ui, int mx, int my) {
  SuwaButton *btn = find_button_at(mx, my);
  if (!btn) return;

  btn->pressed = 0;

  XftDraw *backdraw = XftDrawCreate(ui->display, ui->backbuf,
      DefaultVisual(ui->display, DefaultScreen(ui->display)),
        DefaultColormap(ui->display, DefaultScreen(ui->display)));
  if (!backdraw) {
    fprintf(stderr, "Pixmap向けXftDrawの作成に失敗。\n");
    XFreePixmap(ui->display, ui->backbuf);
    ui->backbuf = None;
    return;
  }

  ui->target = ui->backbuf;
  drawbuttons(ui, btn, backdraw);
  XftDrawDestroy(backdraw);
  XFlush(ui->display);

  const char *label = btn->label;
  if (strcmp(label, "C") == 0) {
    clear_calculator();
  } else if (strcmp(label, "<") == 0) {
    curinput[--input_pos] = '\0';
    strcpy(displaytxt, curinput[0] ? curinput : "0");
  } else if (strcmp(label, "=") == 0) {
    double res = evaluate_simple(curinput);
    if (isnan(res)) {
      strncpy(displaytxt, "Error", 5);
    } else {
      snprintf(displaytxt, sizeof(displaytxt), "%.8g", res);
      strncpy(curinput, displaytxt, strlen(displaytxt));
      input_pos = strlen(curinput);
    }
  } else if (strlen(label) == 1) {
    append_to_input(label[0]);
  }

  control_expose(ui, &(XEvent){.type = Expose});
}

void handle_key_press(UiSystem *ui, XEvent *event) {
  KeySym keysym;
  char buf[32];
  int len;
  (void)len;

  keysym = XLookupKeysym(&event->xkey, 0);
  len = XLookupString(&event->xkey, buf, sizeof(buf), &keysym, NULL);

  if (keysym == XK_Q) {
    isrunning = 0;
    return;
  }

  if (keysym == XK_B) {
    puts("標準電卓画面");
  }

  if (keysym == XK_S) {
    puts("関数電卓画面");
  }

  if (keysym == XK_P) {
    puts("プログラマー電卓画面");
  }

  if (keysym == XK_H) {
    puts("履歴画面");
  }

  if (keysym == XK_h) {
    puts("ヘルプ画面");
    puts("-----");
    puts("Q = 終了 Exit");
    puts("X = １文字の消し Remove 1 character");
    puts("C = クリ了 Clear");
    puts("h = ヘルプ画面 Help screen");
    puts("H = 履歴画面 History screen");
    puts("B = 標準電卓画面 Basic calculator screen");
    puts("S = 関数電卓画面 Scientific calculator screen");
    puts("P = プログラマー電卓画面 Programmer calculator screen");
  }

  if (keysym == XK_C) {
    clear_calculator();
    goto redraw;
  }

  if (keysym == XK_Return || keysym == XK_KP_Enter) {
    double res = evaluate_simple(curinput);
    if (isnan(res)) {
      strncpy(displaytxt, "Error", 5);
    } else {
      snprintf(displaytxt, sizeof(displaytxt), "%.8g", res);
    }
    goto redraw;
  }

  if ((keysym == XK_BackSpace && input_pos > 0)
  || (keysym == XK_Delete && input_pos > 0)
  || (keysym == XK_X && input_pos > 0)) {
    curinput[--input_pos] = '\0';
    strcpy(displaytxt, curinput[0] ? curinput : "0");
    goto redraw;
  }

  char c = 0;
  if      (keysym >= XK_0 && keysym <= XK_9) c = '0' + (keysym - XK_0);
  else if (keysym >= XK_KP_0 && keysym <= XK_KP_9) c = '0' + (keysym - XK_0);
  else if (keysym == XK_KP_Add      || keysym == XK_plus)       c = '+';
  else if (keysym == XK_KP_Subtract || keysym == XK_minus)      c = '-';
  else if (keysym == XK_KP_Multiply || keysym == XK_asterisk)   c = '*';
  else if (keysym == XK_KP_Divide   || keysym == XK_slash)      c = '/';
  else if (keysym == XK_KP_Decimal  || keysym == XK_period)     c = '.';

  if (c) {
    append_to_input(c);
    goto redraw;
  }

  return;

redraw:
  control_expose(ui, &(XEvent){.type = Expose});
}

// void handle_mouse_hover(UiSystem *ui, XEvent *event) {
//   SuwaButton *hover = find_button_at(event->xmotion.x, event->xmotion.y);

//   if (hover != hovered_btn) {
//     if (hovered_btn) {
//       hovered_btn->pressed = 0;
//       redraw_single_button(&ui, hovered_btn);
//     }
//     hovered_btn = hover;

//     if (hovered_btn) {
//       redraw_single_button(&ui, hovered_btn);
//     }
//   }
// }
