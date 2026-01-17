#include <X11/keysym.h>
#include <math.h>
#include <ctype.h>

#include "control.h"
#include "display.h"

char curinput[64] = {0};
int input_pos = 0;
int initialized = 0;
XftColor buttonColor;

static const char *btn_labels[][10] = {
  { NULL }, // 数字表示
  { "C", "±", "%", "÷" },
  { "7", "8", "9", "×" },
  { "4", "5", "6", "-" },
  { "1", "2", "3", "+" },
  { "0", ".", "=", "<" },
};

#define NUM_ROWS 6
#define NUM_COLS 4

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
  found.text = btn_labels[row][col];
  found.pressed = 1;

  return &found;
}

void append_to_input(UiSystem *ui, char c) {
  if ((unsigned long)input_pos >= sizeof(curinput) - 2) return;
  strcpy(ui->problemLabel.text, "");
  curinput[input_pos++] = c;
  curinput[input_pos] = '\0';

  strncpy(ui->resLabel.text, curinput, sizeof(ui->resLabel.text) - 1);
  ui->resLabel.text[sizeof(ui->resLabel.text) - 1] = '\0';
}

void clear_calculator(UiSystem *ui) {
  strcpy(ui->problemLabel.text, "");
  curinput[0] = '\0';
  input_pos = 0;
  strcpy(ui->resLabel.text, "0");
}

double evaluate_simple(UiSystem *ui, const char *expr) {
  double res = 0.0;
  double cur = 0.0;
  char op = '+';
  int i = 0;
  snprintf(ui->problemLabel.text, sizeof(ui->problemLabel.text), "%s=", expr);

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
  XGetWindowAttributes(ui->display, ui->xwindow, &attr);
  int w = attr.width;
  int h = attr.height;

  if (ui->backbuf != None) {
    XFreePixmap(ui->display, ui->backbuf);
    ui->backbuf = None;
  }

  ui->backbuf = XCreatePixmap(ui->display, ui->xwindow, w, h,
      DefaultDepth(ui->display, DefaultScreen(ui->display)));

  if (ui->backbuf == None) {
    fprintf(stderr, "バックバッファ作成失敗！\n");
    ui->backbuf = ui->xwindow;
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
  {
    XGlyphInfo extents;
    XftTextExtentsUtf8(ui->display, ui->resLabel.font,
        (const FcChar8 *)ui->resLabel.text,
        strlen(ui->resLabel.text), &extents);

    int tx = w - 20 - extents.xOff;

    XftDrawStringUtf8(
        backdraw, &ui->resLabel.fg_color, ui->resLabel.font,
        tx, ui->resLabel.y,
        (const FcChar8 *)ui->resLabel.text, 32);
  }

  {
    XGlyphInfo extents;
    XftTextExtentsUtf8(ui->display, ui->problemLabel.font,
        (const FcChar8 *)ui->problemLabel.text,
        strlen(ui->problemLabel.text), &extents);

    int tx = w - 20 - extents.xOff;

    XftDrawStringUtf8(
        backdraw, &ui->problemLabel.fg_color,
        ui->problemLabel.font, tx, ui->problemLabel.y,
        (const FcChar8 *)ui->problemLabel.text, 32);
  }

  for (int i = 0; i < 64; ++i) ui->buttons[i] = NULL;
  int width = 93;
  int height = 60;
  int padding = 2;
  printf("ウィンドウ: (%dx%d)\n", attr.width, attr.height);

  if (initialized == 0) {
    XftColorAllocName(ui->display,
      DefaultVisual(ui->display, DefaultScreen(ui->display)),
      DefaultColormap(ui->display, DefaultScreen(ui->display)),
      "#232020", &buttonColor);
  }
  initialized = 1;

  for (int row = 1; row < NUM_ROWS; ++row) {
    int y = 162 + row * (height + padding);

    for (int col = 0; col < NUM_COLS; ++col) {
      const char *label = btn_labels[row][col];
      if (!label) continue;

      int x = padding + col * (width + padding);

      SuwaButton *btn = malloc(sizeof(SuwaButton));
      btn->x = x;
      btn->y = y;
      btn->width = width;
      btn->height = height;
      btn->text = label;
      btn->bg_color = BTCOL;
      btn->fg_color = buttonColor;
      btn->pressed = 0;
      // ui->buttons[0] = btn;

      drawbuttons(ui, btn, backdraw);
    }
  }

  XCopyArea(ui->display, ui->backbuf, ui->xwindow, ui->gc, 0, 0, w, h, 0, 0);
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

  const char *label = btn->text;
  if (strcmp(label, "C") == 0) {
    clear_calculator(ui);
  } else if (strcmp(label, "<") == 0) {
    curinput[--input_pos] = '\0';
    strcpy(ui->resLabel.text, curinput[0] ? curinput : "0");
  } else if (strcmp(label, "=") == 0) {
    double res = evaluate_simple(ui, curinput);
    if (isnan(res)) {
      strncpy(ui->resLabel.text, "Error", 5);
    } else {
      snprintf(ui->resLabel.text, sizeof(ui->resLabel.text), "%.8g", res);
      strncpy(curinput, ui->resLabel.text, strlen(ui->resLabel.text));
      input_pos = strlen(curinput);
      strcpy(curinput, "");
      curinput[0] = '\0';
      input_pos = 0;
    }
  } else if (strlen(label) == 1) {
    append_to_input(ui, label[0]);
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
    ui->isrunning = 0;
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
    clear_calculator(ui);
    goto redraw;
  }

  if (keysym == XK_Return || keysym == XK_KP_Enter) {
    double res = evaluate_simple(ui, curinput);
    if (isnan(res)) {
      strncpy(ui->resLabel.text, "Error", 5);
    } else {
      snprintf(ui->resLabel.text, sizeof(ui->resLabel.text), "%.8g", res);
    }
    goto redraw;
  }

  if ((keysym == XK_BackSpace && input_pos > 0)
  || (keysym == XK_Delete && input_pos > 0)
  || (keysym == XK_X && input_pos > 0)) {
    curinput[--input_pos] = '\0';
    strcpy(ui->resLabel.text, curinput[0] ? curinput : "0");
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
    append_to_input(ui, c);
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
