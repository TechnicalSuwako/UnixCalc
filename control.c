/**************************************************************************************
# 076 License

Copyright (c) 2026 テクニカル諏訪子

Permission is hereby granted to any person obtaining a copy of the software
UnixCalc (the "Software") to use, modify, merge, copy, publish, distribute,
sublicense, and/or sell copies of the Software, subject to the following conditions:

    1. **Origin Attribution**:
       - You must not misrepresent the origin of the Software; you must not claim
         you created the original Software.
       - If the Software is used in a product, you must either:
         a. Provide clear attribution in the product's documentation, user interface,
            or other visible areas, **OR**
         b. Pay the original developers a fee they specify in writing.
    2. **Usage Restriction**:
       - The Software, or any derivative works, dependencies, or libraries
         incorporating it, must not be used for censorship or to suppress freedom of
         speech, expression, or creativity. Prohibited uses include, but are not
         limited to:
         - Censorship of so-called "hate speech", visuals, non-mainstream opinions,
           ideas, or objective reality.
         - Tools or systems designed to restrict access to information or
           artistic works.
    3. **Notice Preservation**:
       - This license and the above copyright notice must remain intact in all copies
         of the source code.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************/
#include <X11/keysym.h>
#include <math.h>
#include <ctype.h>

#include "control.h"

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
SuwaButton *lasthover = NULL;

static const char *btn_labels[][10] = {
  { NULL }, // 数字表示
  { "C", "±", "%", "÷" },
  { "7", "8", "9", "×" },
  { "4", "5", "6", "-" },
  { "1", "2", "3", "+" },
  { "0", ".", "=", "<" },
};

static SuwaButton *find_button_at(SuwaWindow *window, int mx, int my) {
  static SuwaButton found = {0};

  float scale = get_scale(window);
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
  found.row     = row;
  found.col     = col;

  return &found;
}

void append_to_input(CtrlLabels *labels, char c) {
  if ((unsigned long)input_pos >= sizeof(curinput) - 2) return;
  strcpy(labels->problem->text, "");
  curinput[input_pos++] = c;
  curinput[input_pos] = '\0';

  strncpy(labels->res->text, curinput, sizeof(labels->res->text) - 1);
  labels->res->text[sizeof(labels->res->text) - 1] = '\0';
}

void clear_calculator(CtrlLabels *labels) {
  strcpy(labels->problem->text, "");
  curinput[0] = '\0';
  input_pos = 0;
  strcpy(labels->res->text, "0");
}

double evaluate_simple(CtrlLabels *labels, const char *expr) {
  double res = 0.0;
  double cur = 0.0;
  char op = '+';
  int i = 0;
  snprintf(labels->problem->text, sizeof(labels->problem->text), "%s=", expr);

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

void initialize_basic_buttons(SuwaWindow *window, XftDraw *backdraw, float scale,
    SuwaButton *but) {
  int bw = (int)(DESIGN_BUTTON_WIDTH  * scale + .5f);
  int bh = (int)(DESIGN_BUTTON_HEIGHT * scale + .5f);
  int bp = (int)(DESIGN_PADDING       * scale + .5f);
  int by = (int)(DESIGN_BUTTON_AREA_Y * scale + .5f);

  XftColor buttonColor;
  buttonColor = suwaui_set_button_fgcolor(window, COLOR_BS);

  for (int row = 1; row < NUM_ROWS; ++row) {
    for (int col = 0; col < NUM_COLS; ++col) {
      SuwaButton btn = suwaui_add_button(bp + col * (bw + bp),
        by + (row - 1) * (bh + bp),
        bw, bh, btn_labels[row][col], window->font, COLOR_ND, buttonColor);

      if (but != NULL && but->text != NULL) {
        if (strcmp(but->text, btn.text) == 0) {
          if (but->pressed == 1) btn.pressed = 1;
          else if (but->hovered == 1) btn.hovered = 1;
        }
      }
      suwaui_draw_button(window, &btn, backdraw);
    }
  }
}

void control_expose(SuwaWindow *window, CtrlLabels *labels, SuwaButton *button) {
  if (window->event.type != Expose && window->event.type != ConfigureNotify) return;

  XWindowAttributes attr;
  XGetWindowAttributes(window->display, window->xwindow, &attr);
  int w = attr.width;
  int h = attr.height;

  if (window->backbuf != None) XFreePixmap(window->display, window->backbuf);
  window->backbuf = XCreatePixmap(window->display, window->xwindow, w, h,
      DefaultDepth(window->display, DefaultScreen(window->display)));

  if (window->backbuf == None) window->backbuf = window->xwindow;
  window->target = window->backbuf;

  XftDraw *backdraw = XftDrawCreate(window->display, window->backbuf,
      DefaultVisual(window->display, DefaultScreen(window->display)),
        DefaultColormap(window->display, DefaultScreen(window->display)));
  if (!backdraw) {
    fprintf(stderr, "Pixmap向けXftDrawの作成に失敗。\n");
    XFreePixmap(window->display, window->backbuf);
    window->backbuf = None;
    return;
  }

  XSetForeground(window->display, window->gc, COLOR_BD);
  XFillRectangle(window->display, window->backbuf, window->gc, 0, 0, w, h);

  float scale = get_scale(window);
  if (scale <= 0) scale = 1.f;

  int label_padding = (int)(20 * scale + .5f);

  // 出力
  {
    XGlyphInfo extents;
    XftTextExtentsUtf8(window->display, labels->res->font,
        (const FcChar8 *)labels->res->text,
        strlen(labels->res->text), &extents);

    int tx = w - label_padding - extents.xOff;
    int ty = (int)(180 * scale + .5f);

    XftDrawStringUtf8(
        backdraw, &labels->res->fg_color, labels->res->font,
        tx, ty, (const FcChar8 *)labels->res->text, 64);
  }

  {
    XGlyphInfo extents;
    XftTextExtentsUtf8(window->display, labels->problem->font,
        (const FcChar8 *)labels->problem->text,
        strlen(labels->problem->text), &extents);

    int tx = w - label_padding - extents.xOff;
    int ty = (int)(80 * scale + .5f);

    XftDrawStringUtf8(
        backdraw, &labels->problem->fg_color, labels->problem->font,
        tx, ty, (const FcChar8 *)labels->problem->text, 64);
  }

  initialize_basic_buttons(window, backdraw, scale, button);

  XCopyArea(window->display, window->backbuf, window->xwindow, window->gc,
      0, 0, w, h, 0, 0);
  XftDrawDestroy(backdraw);
  XFlush(window->display);
}

void handle_button_press(SuwaWindow *window, CtrlLabels *labels, int mx, int my) {
  SuwaButton *btn = find_button_at(window, mx, my);
  if (!btn) return;

  btn->pressed = 1;
  window->event = (XEvent){.type = Expose};
  control_expose(window, labels, btn);
}

void handle_button_release(SuwaWindow *window, CtrlLabels *labels, int mx, int my) {
  SuwaButton *btn = find_button_at(window, mx, my);
  if (!btn) return;

  btn->pressed = 0;
  const char *label = btn->text;
  if (strcmp(label, "C") == 0) {
    clear_calculator(labels);
  } else if (strcmp(label, "×") == 0) {
    append_to_input(labels, '*');
  } else if (strcmp(label, "÷") == 0) {
    append_to_input(labels, '/');
  } else if (strcmp(label, "<") == 0) {
    curinput[--input_pos] = '\0';
    strcpy(labels->res->text, curinput[0] ? curinput : "0");
  } else if (strcmp(label, "=") == 0) {
    double res = evaluate_simple(labels, curinput);
    if (isnan(res)) strncpy(labels->res->text, "Error", 5);
    else snprintf(labels->res->text, sizeof(labels->res->text), "%.8g", res);
  } else if (strlen(label) == 1) {
    append_to_input(labels, label[0]);
  }

  window->event = (XEvent){.type = Expose};
  control_expose(window, labels, btn);
}

void handle_key_press(SuwaWindow *window, CtrlLabels *labels) {
  KeySym keysym;
  char buf[32];
  int len;
  (void)len;

  keysym = XLookupKeysym(&window->event.xkey, 0);
  len = XLookupString(&window->event.xkey, buf, sizeof(buf), &keysym, NULL);

  if (keysym == XK_Q) {
    window->isrunning = 0;
    return;
  }

  if (keysym == XK_B) {
    puts("標準電卓画面");
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
    puts("P = プログラマー電卓画面 Programmer calculator screen");
  }

  if (keysym == XK_C) {
    clear_calculator(labels);
    goto redraw;
  }

  if (keysym == XK_Return || keysym == XK_KP_Enter) {
    double res = evaluate_simple(labels, curinput);
    if (isnan(res)) strncpy(labels->res->text, "Error", 5);
    else snprintf(labels->res->text, sizeof(labels->res->text), "%.8g", res);
    goto redraw;
  }

  if ((keysym == XK_BackSpace && input_pos > 0)
  || (keysym == XK_Delete && input_pos > 0)
  || (keysym == XK_X && input_pos > 0)) {
    curinput[--input_pos] = '\0';
    strcpy(labels->res->text, curinput[0] ? curinput : "0");
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
    append_to_input(labels, c);
    goto redraw;
  }

  return;

redraw:
  window->event = (XEvent){.type = Expose};
  control_expose(window, labels, &(SuwaButton){0});
}

void handle_mouse_hover(SuwaWindow *window, CtrlLabels *labels, int mx, int my) {
  SuwaButton *btn = find_button_at(window, mx, my);
  if (!btn) return;

  if (lasthover && lasthover != btn) lasthover->hovered = 0;
  if (btn) btn->hovered = 1;
  lasthover = btn;

  window->event = (XEvent){.type = Expose};
  control_expose(window, labels, btn ? btn : &(SuwaButton){0});
}
