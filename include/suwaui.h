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
#pragma once

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/Xatom.h>

#include <stdio.h>

#define FGCOL 0xfcfcfc
#if defined(__OpenBSD__)
#define BGCOL 0x232320
#define BTSEL 0xb8b515
#define BTCOL 0xf1ed25
#define BTHVR 0xecea71
#elif defined(__FreeBSD__)
#define BGCOL 0x232020
#define BTSEL 0xb61729
#define BTCOL 0xee4030
#define BTHVR 0xf35869
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
} SuwaWindow;

__attribute__((unused)) static void suwaui_destroy_window(SuwaWindow *w);

__attribute__((unused)) static inline float get_scale(SuwaWindow *w) {
  XWindowAttributes attr;
  XGetWindowAttributes(w->display, w->xwindow, &attr);
  float sx = (float)attr.width / w->start_width;
  float sy = (float)attr.height / w->start_height;
  return sx < sy ? sx : sy;
}

__attribute__((unused)) static SuwaWindow suwaui_create_window(int x, int y, int width, int height,
    const char *software_name, const char *software_version,
    const char *res_name, const char *class_name,
    const char *font, const char *fg_color,
    int is_centered, int cannot_shrink, int cannot_grow) {
  (void)cannot_grow;

  SuwaWindow w = {0};
  w.width = width;
  w.height = height;
  w.start_width = width;
  w.start_height = height;
  w.name = software_name;
  w.version = software_version;
  w.res_name = res_name;
  w.class_name = class_name;
  w.isrunning = 1;

  XGCValues values;

  w.display = XOpenDisplay(NULL);
  if (w.display == NULL) {
    fprintf(stderr, "screen open fail\n");
    return (SuwaWindow){0};
  }

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
      1, BTCOL, BGCOL);
  if (!w.xwindow) {
    suwaui_destroy_window(&w);
    fprintf(stderr, "window create fail\n");
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

  XSetWindowBackground(w.display, w.xwindow, BGCOL);

  XSelectInput(w.display, w.xwindow,
      ExposureMask
    | ButtonPressMask
    | ButtonReleaseMask
    | KeyPressMask
    // | PointerMotionMask
    // | ButtonMotionMask
    // | StructureNotifyMask
  );

  w.gc = XCreateGC(w.display, w.xwindow, 0, &values);
  if (!w.gc) {
    suwaui_destroy_window(&w);
    fprintf(stderr, "gc create fail\n");
    return (SuwaWindow){0};
  }

  w.visual = *DefaultVisual(w.display, w.screen);

  w.colormap = XCreateColormap(w.display, w.xwindow,
      &w.visual, AllocNone);
  if (w.colormap == None) {
    suwaui_destroy_window(&w);
    fprintf(stderr, "calor map create fail\n");
    return (SuwaWindow){0};
  }

  w.font = XftFontOpenName(w.display, w.screen, font);
  if (!w.font) {
    suwaui_destroy_window(&w);
    fprintf(stderr, "font create fail\n");
    return (SuwaWindow){0};
  }

  if (!XftColorAllocName(w.display, &w.visual,
        w.colormap, fg_color, &w.color)) {
    suwaui_destroy_window(&w);
    fprintf(stderr, "color estimate fail\n");
    return (SuwaWindow){0};
  }

  return w;
}

__attribute__((unused)) static void suwaui_destroy_window(SuwaWindow *w) {
  if (w->font) XftFontClose(w->display, w->font);

  if (w->gc) XFreeGC(w->display, w->gc);
  if (w->backbuf) {
    XFreePixmap(w->display, w->backbuf);
    w->backbuf = None;
  }
  if (w->xwindow) XDestroyWindow(w->display, w->xwindow);
  if (w->display) XCloseDisplay(w->display);
}
#endif // SUWAUI_IMPLEMENTS_SUWAWINDOW

#if defined(SUWAUI_IMPLEMENTS_SUWATABS)
typedef struct {
  int x, y, width, height;
} SuwaTab;

typedef struct {
  int x, y, width, height;
} SuwaTabs;
#endif // SUWAUI_IMPLEMENTS_SUWATABS

#if defined(SUWAUI_IMPLEMENTS_SUWABUTTON)
typedef struct {
  int x, y, width, height;
  const char *text;
  XftFont *font;
  int bg_color;
  XftColor fg_color;
  int pressed; // 0 = 普通、1 = 押している
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
  unsigned long curbg = btn->pressed ? BTSEL : btn->bg_color;
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
    const char *text, XftFont *font, int bg_color, XftColor fg_color) {
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
