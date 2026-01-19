/**************************************************************************************
# 076 Free License

Copyright (c) 2026 テクニカル諏訪子

Permission is hereby granted to any person obtaining a copy of the software
SuwaUI (the "Software") to use, modify, merge, copy, publish, distribute,
sublicense, and/or sell copies of the Software, subject to the following conditions:

    1. **Origin Attribution**:
       - You must not misrepresent the origin of the Software; you must not claim
         you created the original Software.
    2. **Notice Preservation**:
       - This license and the above copyright notice must remain intact in all copies
         of the source code.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
**************************************************************************************/
#ifndef SUWAUI_H
#define SUWAUI_H

#if defined(_WIN32) || defined(_WIN64)
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#else
#define WIN64_LEAN_AND_MEAN
#endif
#include <windows.h>
#define SUWAUI_WINDOWS
#elif defined(__unix__) || defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sunos) || defined(__apple__)
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/Xatom.h>
#define SUWAUI_UNIX
#endif

#include <stdio.h>

#define COLOR_WHITE 0xfcfcfc
#if defined(__OpenBSD__)
#define COLOR_BD 0x232320  // color black decimal
#define COLOR_BS "#232320" // color black string
#define COLOR_DD 0xb8b515  // color dark decimal
#define COLOR_DS "#b8b515" // color dark string
#define COLOR_ND 0xf1ed25  // color normal decimal
#define COLOR_NS "#f1ed25" // color normal string
#define COLOR_LD 0xecea71  // color light decimal
#define COLOR_LS "#ecea71" // color light string
#elif defined(__FreeBSD__)
#define COLOR_BD 0x232020  // color black decimal
#define COLOR_BS "#232020" // color black string
#define COLOR_DD 0xb61729  // color dark decimal
#define COLOR_DS "#b61729" // color dark string
#define COLOR_ND 0xee4030  // color normal decimal
#define COLOR_NS "#ee4030" // color normal string
#define COLOR_LD 0xf35869  // color light decimal
#define COLOR_LS "#f35869" // color light string
#elif defined(__NetBSD__)
#define COLOR_BD 0x232018  // color black decimal
#define COLOR_BS "#232018" // color black string
#define COLOR_DD 0xac7718  // color dark decimal
#define COLOR_DS "#ac7718" // color dark string
#define COLOR_ND 0xf7a717  // color normal decimal
#define COLOR_NS "#f7a717" // color normal string
#define COLOR_LD 0xf8c56a  // color light decimal
#define COLOR_LS "#f8c56a" // color light string
#elif defined(SUWAUI_WINDOWS)
#define COLOR_BD 0x202023  // color black decimal
#define COLOR_BS "#202023" // color black string
#define COLOR_DD 0x1a6ecf  // color dark decimal
#define COLOR_DS "#1a6ecf" // color dark string
#define COLOR_ND 0x2687f7  // color normal decimal
#define COLOR_NS "#2687f7" // color normal string
#define COLOR_LD 0x6aa6eb  // color light decimal
#define COLOR_LS "#6aa6eb" // color light string
#elif defined(__apple__)
#define COLOR_BD 0x232323  // color black decimal
#define COLOR_BS "#232323" // color black string
#define COLOR_DD 0x746c74  // color dark decimal
#define COLOR_DS "#746c74" // color dark string
#define COLOR_ND 0x988f98  // color normal decimal
#define COLOR_NS "#988f98" // color normal string
#define COLOR_LD 0xbdb4bd  // color light decimal
#define COLOR_LS "#bdb4bd" // color light string
#elif defined(__sunos)
#define COLOR_BD 0x182023  // color black decimal
#define COLOR_BS "#182023" // color black string
#define COLOR_DD 0x1cbcd0  // color dark decimal
#define COLOR_DS "#1cbcd0" // color dark string
#define COLOR_ND 0x29d3ff  // color normal decimal
#define COLOR_NS "#29d3ff" // color normal string
#define COLOR_LD 0x8ae5ff  // color light decimal
#define COLOR_LS "#8ae5ff" // color light string
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(SUWAUI_IMPLEMENTS_SUWAWINDOW)
#include <string.h>

typedef struct {
  int x, y, width, height;
  int start_width, start_height;
  int isrunning;
  const char *name;
  const char *version;
#if defined(SUWAUI_WINDOWS)
  HWND hwnd;
  HDC hdc_mem;
  HBITMAP backbitmap;
  HFONT hfont;
  COLORREF fg_color;
#elif defined(SUWAUI_UNIX)
  const char *res_name;
  const char *class_name;
  Display *display;
  Window xwindow;
  int screen;
  Drawable target;
  GC gc;
  Visual visual;
  XftColor color;
  XftFont *font;
  Colormap colormap;
  Pixmap backbuf;
  XEvent event;
#endif
} SuwaWindow;

__attribute__((unused)) static void suwaui_destroy_window(SuwaWindow *w);

__attribute__((unused)) static inline float get_scale(SuwaWindow *w) {
#if defined(SUWAUI_WINDOWS)
#elif defined(SUWAUI_UNIX)
  XWindowAttributes attr;
  XGetWindowAttributes(w->display, w->xwindow, &attr);
  float sx = (float)attr.width / w->start_width;
  float sy = (float)attr.height / w->start_height;
#endif
  return sx < sy ? sx : sy;
}

#if defined(SUWAUI_WINDOWS)
LRESULT CALLBACK SuwaWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  SuwaWindow *w = (SuwaWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

  switch (msg) {
    case WM_CREATE:
      LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
      w = (SuwaWindow *)cs->lpCreateParams;
      SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)w);
      return 0
    case WM_SIZE:
        if (w == NULL) break;

        int new_w = LOWORD(lParam);
        int new_h = HIWORD(lParam);

        if (w->back_bitmap != NULL) {
          DeleteObject(w->back_bitmap);
          w->back_bitmap = NULL;
        }

        HDC hdc = GetDC(hwnd);
        if (hdc) {
          w->back_bitmap = CreateCompatibleBitmap(hdc, new_w, new_h);
          if (w->back_bitmap) {
            SelectObject(w->hdc_memm, w->back_bitmap);
          }
          ReleaseDC(hwnd, hdc);
        }

        w->width = new_w;
        w->height = new_h;

        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    case WM_PAINT:
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        if (w && w->hdc_mem) {
          RECT client;
          GetClientRect(hwnd, &client);
          FillRect(w->hdc_mem, &client, (HBRUSH)GetStockObject(COLOR_BD));

          // suwaui_on_draw(w);

          BitBlt(hdc, 0, 0, w->width, w->height, w->hdc_mem, 0, 0, SRCCOPY);
        }

        EndPaint(hwnd, &ps);
        return 0;
    // case WM_ERASEBKGND:
    //     return 1;
    // case WM_LBUTTONDOWN:
    // case WM_LBUTTONUP:
    // case WM_MOUSEMOVE:
    // case WM_KEYDOWN:
    // case WM_KEYUP:
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
  }

  return DefWindowProc(hwnd, msg, wParam, lParam);
}
#endif

__attribute__((unused)) static SuwaWindow suwaui_create_window(int x, int y, int width, int height,
    const char *software_name, const char *software_version,
    const char *res_name, const char *class_name,
    const char *font, const char *fg_color,
    int is_centered, int cannot_shrink, int cannot_grow) {
  (void)cannot_grow;

  SuwaWindow w = {0};
  w.isrunning = 1;
  w.width = width;
  w.height = height;
  w.start_width = width;
  w.start_height = height;
  w.name = software_name;
  w.version = software_version;
#if defined(SUWAUI_WINDOWS)
  WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = SuwaWindowsProc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.hCursor = LoadCursor(NULL, IDCARROW);
  wc.hbrBackground = NULL;
  wc.lpszClassName = TEXT("SuwaWindowClass");

  RegisterClassEx(&wc);

  DWORD style = WS_OVERLAPPEDWINDOW;

  w.hwnd = CreateWindowEx(0, wc.lpszClassName, name, style,
      x, y, width, height, NULL, NULL, wc.hInstance, NULL);
  if (!w.hwnd) return (SUwaWindow){0};

  HDC hdc = GetDC(w.hwnd);
  w.hdc_mem = CreateCompatibleDC(hdc);
  w.back_bitmap = CreateCompatibleBitmap(hdc, width, height);
  SelectObject(w.hdc_mem, w.back_bitmap);
  ReleaseDC(w.hwnd, hdc);

  ShowWindow(w.hwnd, hdc);
  UpdateWindow(w.hwnd);
#elif defined(SUWAUI_UNIX)
  w.res_name = res_name;
  w.class_name = class_name;

  XGCValues values;

  w.display = XOpenDisplay(NULL);
  if (w.display == NULL) return (SuwaWindow){0};

  w.screen = DefaultScreen(w.display);

  if (is_centered > 0) {
    int sw = DisplayWidth(w.display, w.screen);
    int sh = DisplayHeight(w.display, w.screen);
    w.x = (sw - w.width) / 2;
    w.y = (sh - w.height) / 2;
  } else {
    w.x = x;
    w.y = y;
  }

  w.xwindow = XCreateSimpleWindow(w.display,
      RootWindow(w.display, w.screen),
      w.x, w.y,
      w.width, w.height,
      1, COLOR_ND, COLOR_BD);
  if (!w.xwindow) {
    suwaui_destroy_window(&w);
    return (SuwaWindow){0};
  }

  if (cannot_shrink) {
    XSizeHints *sizeHint = XAllocSizeHints();
    if (sizeHint) {
      sizeHint->flags = PMinSize;
      sizeHint->min_width = width;
      sizeHint->min_height = height;
      XSetWMNormalHints(w.display, w.xwindow, sizeHint);
     XFree(sizeHint);
    }
  }

  w.backbuf = XCreatePixmap(w.display, w.xwindow,
      w.width, w.height,
      DefaultDepth(w.display, w.screen));
  w.target = w.backbuf;

  Atom net_wm_window_type = XInternAtom(w.display, "_NET_WM_WINDOW_TYPE", False);
  Atom dialog = XInternAtom(w.display, "_NET_WM_WINDOW_TYPE_DIALOG", False);

  XChangeProperty(w.display, w.xwindow, net_wm_window_type, XA_ATOM, 32,
      PropModeReplace, (unsigned char *)&dialog, 1);

  XStoreName(w.display, w.xwindow, w.name);
  Atom net_wm_name = XInternAtom(w.display, "_NET_WM_NAME", False);
  char displayname[16];
  snprintf(displayname, 16, "%s %s", w.name, w.version);
  XChangeProperty(w.display, w.xwindow, net_wm_name,
      XInternAtom(w.display, "UTF8_STRING", False), 8,
      PropModeReplace, (unsigned char *)displayname, strlen(displayname));

  XClassHint *classHint = XAllocClassHint();
  if (classHint) {
    classHint->res_name = strdup(w.res_name);
    classHint->res_class = strdup(w.class_name);
    XSetClassHint(w.display, w.xwindow, classHint);
    XFree(classHint);
  }

  XSetWindowBackground(w.display, w.xwindow, COLOR_BD);

  XSelectInput(w.display, w.xwindow,
      ExposureMask
    | ButtonPressMask
    | ButtonReleaseMask
    | KeyPressMask
    | PointerMotionMask
    | ButtonMotionMask
    // | StructureNotifyMask
  );

  w.gc = XCreateGC(w.display, w.xwindow, 0, &values);
  if (!w.gc) {
    suwaui_destroy_window(&w);
    return (SuwaWindow){0};
  }

  w.visual = *DefaultVisual(w.display, w.screen);

  w.colormap = XCreateColormap(w.display, w.xwindow,
      &w.visual, AllocNone);
  if (w.colormap == None) {
    suwaui_destroy_window(&w);
    return (SuwaWindow){0};
  }

  w.font = XftFontOpenName(w.display, w.screen, font);
  if (!w.font) {
    suwaui_destroy_window(&w);
    return (SuwaWindow){0};
  }

  if (!XftColorAllocName(w.display, &w.visual,
        w.colormap, fg_color, &w.color)) {
    suwaui_destroy_window(&w);
    return (SuwaWindow){0};
  }
#endif

  return w;
}

__attribute__((unused)) static void suwaui_destroy_window(SuwaWindow *w) {
#if defined(SUWAUI_WINDOWS)
  if (w->back_bitmap) DeleteObject(w->back_bitmap);
  if (w->hdc_mem) DeleteDC(w->hdc_mem);
  if (w->hwnd) DestroyWindow(w->hwnd);
#elif defined(SUWAUI_UNIX)
  if (w->font) XftFontClose(w->display, w->font);

  if (w->gc) XFreeGC(w->display, w->gc);
  if (w->backbuf) {
    XFreePixmap(w->display, w->backbuf);
    w->backbuf = None;
  }
  if (w->xwindow) XDestroyWindow(w->display, w->xwindow);
  if (w->display) XCloseDisplay(w->display);
#endif

  *w = (SuwaWindow){0};
}
#endif // SUWAUI_IMPLEMENTS_SUWAWINDOW

#if defined(SUWAUI_IMPLEMENTS_SUWABUTTON)
typedef struct {
  int x, y, width, height;
  int row, col;
  const char *text;
  XftFont *font;
  unsigned long bg_color;
  XftColor fg_color;
  int pressed; // 0 = 普通、1 = 押している
  int hovered;
} SuwaButton;

// __attribute__((unused)) static void suwaui_del_button(SuwaWindow *window, SuwaButton *button);

__attribute__((unused)) static XftColor suwaui_set_button_fgcolor(SuwaWindow *window, const char *color) {
  XftColor xcolor;
  XftColorAllocName(window->display,
    DefaultVisual(window->display, DefaultScreen(window->display)),
    DefaultColormap(window->display, DefaultScreen(window->display)),
    color, &xcolor);

  return xcolor;
}

__attribute__((unused)) static void suwaui_draw_button(SuwaWindow *window, SuwaButton *btn, XftDraw *xftdraw) {
  unsigned long curbg;
  if (btn->pressed) curbg = COLOR_DD;
  else if (btn->hovered) curbg = COLOR_LD;
  else curbg = btn->bg_color;
  XSetForeground(window->display, window->gc, curbg);
  XFillRectangle(window->display, window->target, window->gc, btn->x, btn->y,
      btn->width, btn->height);

  // 文字の中央に
  if (btn->text && window->font && xftdraw) {
    const FcChar8 *str = (const FcChar8 *)btn->text;
    int len = strlen((const char *)str);

    XGlyphInfo extents;
    XftTextExtentsUtf8(window->display, window->font, str, len, &extents);

    int text_w = extents.xOff;
    int text_h = window->font->ascent + window->font->descent;

    int tx = btn->x + (btn->width - text_w) / 2;
    int ty = btn->y + (btn->height - text_h) / 2 + window->font->ascent;

    XftDrawStringUtf8(xftdraw, &btn->fg_color, window->font, tx, ty,
        (FcChar8 *)btn->text, len);
  }
}

__attribute__((unused)) static SuwaButton suwaui_add_button(int x, int y, int width, int height,
    const char *text, XftFont *font, unsigned long bg_color, XftColor fg_color) {
  SuwaButton button = {
    .x        = x,
    .y        = y,
    .width    = width,
    .height   = height,
    .text     = text,
    .font     = font,
    .bg_color = bg_color,
    .fg_color = fg_color,
    .pressed  = 0
  };

  return button;
}
#endif // SUWAUI_IMPLEMENTS_SUWABUTTON

#if defined(SUWAUI_IMPLEMENTS_SUWALABEL)
typedef struct {
  int x, y, width, height;
  char text[64];
  XftFont *font;
  XftColor fg_color;
} SuwaLabel;

__attribute__((unused)) static void suwaui_del_label(SuwaWindow *window, SuwaLabel *label);

__attribute__((unused)) static SuwaLabel suwaui_add_label(SuwaWindow *window,
    int x, int y, int width, int height, char text[64], const char *font,
    const char *fg_color, int rtl) {
  (void)width;
  (void)height;
  SuwaLabel label = {
    .x = x,
    .y = y,
    .width = width,
    .height = height
  };

  label.font = XftFontOpenName(window->display, window->screen, font);
  if (!label.font) {
    fprintf(stderr, "font create fail\n");
    return (SuwaLabel){0};
  }

  strncpy(label.text, text ? text : "", sizeof(label.text) - 1);
  label.text[sizeof(label.text)-1] = '\0';

  const FcChar8 *txt = (const FcChar8 *)label.text;
  int len = strlen((const char *)txt);

  XGlyphInfo extents;
  XftTextExtentsUtf8(window->display, label.font, txt, len, &extents);

  if (rtl > 0) {
    XWindowAttributes attr;
    XGetWindowAttributes(window->display, window->xwindow, &attr);
    int w = attr.width;
    label.x = w - label.x - extents.xOff;
  }

  if (!XftColorAllocName(window->display,
      DefaultVisual(window->display, DefaultScreen(window->display)),
      DefaultColormap(window->display, DefaultScreen(window->display)),
      fg_color, &label.fg_color)) {
    suwaui_del_label(window, &label);
    fprintf(stderr, "color estimate fail\n");
    return (SuwaLabel){0};
  }

  return label;
}

__attribute__((unused)) static void suwaui_del_label(SuwaWindow *window, SuwaLabel *label) {
  if (label->font) XftFontClose(window->display, label->font);
}
#endif // SUWAUI_IMPLEMENTS_SUWALABEL

#if defined(__cplusplus)
}
#endif

#if defined(SUWAUI_IMPLEMENTS_SUWATABS)
typedef struct {
  int x, y, width, height;
} SuwaTab;

typedef struct {
  int x, y, width, height;
} SuwaTabs;
#endif // SUWAUI_IMPLEMENTS_SUWATABS

#if defined(SUWAUI_IMPLEMENTS_SUWASIDE)
typedef struct {
  int x, y, width, height;
} SuwaSide;
#endif // SUWAUI_IMPLEMENTS_SUWASIDE

#if defined(SUWAUI_IMPLEMENTS_SUWATEXTBOX)
typedef struct {
  int x, y, width, height;
} SuwaTextbox;
#endif // SUWAUI_IMPLEMENTS_SUWATEXTBOX

#if defined(SUWAUI_IMPLEMENTS_SUWATEXTEDIT)
typedef struct {
  int x, y, width, height;
} SuwaTextEdit;
#endif // SUWAUI_IMPLEMENTS_SUWATEXTEDIT

#if defined(SUWAUI_IMPLEMENTS_SUWAMENUBAR)
typedef struct {
  int x, y, width, height;
} SuwaMenubar;
#endif // SUWAUI_IMPLEMENTS_SUWAMENUBAR

#if defined(SUWAUI_IMPLEMENTS_SUWADIALOG)
typedef struct {
  int x, y, width, height;
} SuwaDialog;
#endif // SUWAUI_IMPLEMENTS_SUWADIALOG

#if defined(SUWAUI_IMPLEMENTS_SUWAIO)
typedef struct {
  int x, y, width, height;
} SuwaIO;
#endif // SUWAUI_IMPLEMENTS_SUWAIO
#endif // SUWAUI_H
