#include <X11/keysym.h>
#include <math.h>
#include <ctype.h>

#include "control.h"
#include "display.h"

#define NUM_ROWS 6
#define NUM_COLS 4

#define DESIGN_WIDTH          382
#define DESIGN_HEIGHT         534

#define DESIGN_BUTTON_AREA_Y  224
#define DESIGN_BUTTON_WIDTH    93
#define DESIGN_BUTTON_HEIGHT   60
#define DESIGN_PADDING          2

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

static inline float get_scale(UiSystem *ui) {
  XWindowAttributes attr;
  XGetWindowAttributes(ui->display, ui->xwindow, &attr);
  float sx = (float)attr.width / DESIGN_WIDTH;
  float sy = (float)attr.height / DESIGN_HEIGHT;
  return sx < sy ? sx : sy;
}

static SuwaButton *find_button_at(UiSystem *ui, int mx, int my) {
  static SuwaButton found = {0};

  float scale = get_scale(ui);
  if (scale <= 0) scale = 1.0f;

  int btn_w =   (int)(DESIGN_BUTTON_WIDTH  * scale + .5f);
  int btn_h =   (int)(DESIGN_BUTTON_HEIGHT * scale + .5f);
  int padding = (int)(DESIGN_PADDING       * scale + .5f);;
  int start_y = (int)(DESIGN_BUTTON_AREA_Y * scale + .5f);;

  int row = -1;
  int col = -1;

  for (int r = 1; r < NUM_ROWS; ++r) {
    int y_start = start_y + (r - 1) * btn_h + padding;
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

  found.x       = padding + col * (btn_w + padding);
  found.y       = start_y + (row - 1) * (btn_h + padding);
  found.width   = btn_w;
  found.height  = btn_h;
  found.text    = btn_labels[row][col];
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

  if (ui->backbuf != None) XFreePixmap(ui->display, ui->backbuf);
  ui->backbuf = XCreatePixmap(ui->display, ui->xwindow, w, h,
      DefaultDepth(ui->display, DefaultScreen(ui->display)));

  if (ui->backbuf == None) ui->backbuf = ui->xwindow;
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

  float scale = get_scale(ui);
  if (scale <= 0) scale = 1.f;

  int label_padding = (int)(20 * scale + .5f);

  // 出力
  {
    XGlyphInfo extents;
    XftTextExtentsUtf8(ui->display, ui->resLabel.font,
        (const FcChar8 *)ui->resLabel.text,
        strlen(ui->resLabel.text), &extents);

    int tx = w - label_padding - extents.xOff;
    int ty = (int)(180 * scale + .5f);

    XftDrawStringUtf8(
        backdraw, &ui->resLabel.fg_color, ui->resLabel.font,
        tx, ty, (const FcChar8 *)ui->resLabel.text, 64);
  }

  {
    XGlyphInfo extents;
    XftTextExtentsUtf8(ui->display, ui->problemLabel.font,
        (const FcChar8 *)ui->problemLabel.text,
        strlen(ui->problemLabel.text), &extents);

    int tx = w - label_padding - extents.xOff;
    int ty = (int)(80 * scale + .5f);

    XftDrawStringUtf8(
        backdraw, &ui->problemLabel.fg_color, ui->problemLabel.font,
        tx, ty, (const FcChar8 *)ui->problemLabel.text, 64);
  }

  int width   = (int)(DESIGN_BUTTON_WIDTH  * scale + .5f);
  int height  = (int)(DESIGN_BUTTON_HEIGHT * scale + .5f);
  int padding = (int)(DESIGN_PADDING       * scale + .5f);
  int start_y = (int)(DESIGN_BUTTON_AREA_Y * scale + .5f);

  if (initialized == 0) {
    XftColorAllocName(ui->display,
      DefaultVisual(ui->display, DefaultScreen(ui->display)),
      DefaultColormap(ui->display, DefaultScreen(ui->display)),
#if defined(__OpenBSD__)
      "#232320", &buttonColor);
#elif defined(__FreeBSD__)
      "#232020", &buttonColor);
#endif
  }
  initialized = 1;

  for (int row = 1; row < NUM_ROWS; ++row) {
    for (int col = 0; col < NUM_COLS; ++col) {
      SuwaButton btn = {
        .x        = padding + col * (width + padding),
        .y        = start_y + (row - 1) * (height + padding),
        .width    = width,
        .height   = height,
        .text     = btn_labels[row][col],
        .font     = ui->font,
        .bg_color = BTCOL,
        .fg_color = buttonColor,
        .pressed  = 0
      };

      drawbuttons(ui, &btn, backdraw);
    }
  }

  XCopyArea(ui->display, ui->backbuf, ui->xwindow, ui->gc, 0, 0, w, h, 0, 0);
  XftDrawDestroy(backdraw);
  XFlush(ui->display);
}

void handle_button_press(UiSystem *ui, int mx, int my) {
  SuwaButton *btn = find_button_at(ui, mx, my);
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
  SuwaButton *btn = find_button_at(ui, mx, my);
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
  } else if (strcmp(label, "×") == 0) {
    append_to_input(ui, '*');
  } else if (strcmp(label, "÷") == 0) {
    append_to_input(ui, '/');
  } else if (strcmp(label, "<") == 0) {
    curinput[--input_pos] = '\0';
    strcpy(ui->resLabel.text, curinput[0] ? curinput : "0");
  } else if (strcmp(label, "=") == 0) {
    double res = evaluate_simple(ui, curinput);
    if (isnan(res)) strncpy(ui->resLabel.text, "Error", 5);
    else snprintf(ui->resLabel.text, sizeof(ui->resLabel.text), "%.8g", res);
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
    if (isnan(res)) strncpy(ui->resLabel.text, "Error", 5);
    else snprintf(ui->resLabel.text, sizeof(ui->resLabel.text), "%.8g", res);
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
