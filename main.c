#include <stdio.h>

#include <X11/Xatom.h>
#include "src/utils.h"
#include "src/control.h"

XftColor color, btncolor;
Colormap colormap;
XftFont *font;
XftFont *prbfont;
XftFont *disfont;
Visual *visual;

const char *sofname = "ucalc";
const char *version = "0.0.0";
const char *disname = "Unix Calc";

int main() {
  Display *display;
  Window window;
  XEvent event;
  int screen;
  GC gc = NULL;
  XGCValues values;

  display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "画面を開けられません。\n");
    exit(1);
  }

  screen = DefaultScreen(display);

  int sw = DisplayWidth(display, screen);
  int sh = DisplayHeight(display, screen);
  int window_x = (sw - window_width) / 3;
  int window_y = (sh - window_height) / 2;

  window = XCreateSimpleWindow(display, RootWindow(display, screen),
      window_x, window_y, window_width, window_height, 1, BTCOL, BGCOL);
  if (!window) {
    cleanup(display, window, gc, &color, &btncolor, font, disfont, prbfont, colormap, visual, backbuf);
    fprintf(stderr, "ウィンドウを作成に失敗。\n");
    exit(1);
  }

  backbuf = XCreatePixmap(display, window, window_width, window_height, DefaultDepth(display, screen));

  Atom net_wm_window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
  Atom dialog = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);

  XChangeProperty(display, window, net_wm_window_type, XA_ATOM, 32,
      PropModeReplace, (unsigned char *)&dialog, 1);

  XStoreName(display, window, disname);
  Atom net_wm_name = XInternAtom(display, "_NET_WM_NAME", False);
  char displayname[16];
  snprintf(displayname, 16, "%s %s", disname, version);
  XChangeProperty(display, window, net_wm_name,
      XInternAtom(display, "UTF8_STRING", False), 8,
      PropModeReplace, (unsigned char *)displayname, strlen(displayname));

  XClassHint *classHint = XAllocClassHint();
  if (classHint) {
    classHint->res_name = "unixcalc";
    classHint->res_class = "UnixCalc";
    XSetClassHint(display, window, classHint);
    XFree(classHint);
  }

  XSetWindowBackground(display, window, BGCOL);

  XSelectInput(display, window,
      ExposureMask
    | ButtonPressMask
    | ButtonReleaseMask
    | KeyPressMask
    // | PointerMotionMask
    // | ButtonMotionMask
    // | StructureNotifyMask
  );

  gc = XCreateGC(display, window, 0, &values);
  if (!gc) {
    cleanup(display, window, gc, &color, &btncolor, font, disfont, prbfont, colormap, visual, backbuf);
    fprintf(stderr, "GCを作成に失敗。\n");
    exit(1);
  }

  visual = DefaultVisual(display, screen);

  colormap = XCreateColormap(display, window, visual, AllocNone);
  if (colormap == None) {
    cleanup(display, window, gc, &color, &btncolor, font, disfont, prbfont, colormap, visual, backbuf);
    fprintf(stderr, "カラーマップを作成に失敗。\n");
    exit(1);
  }

  font = XftFontOpenName(display, screen, "Noto Sans CJK-12");
  if (!font) {
    cleanup(display, window, gc, &color, &btncolor, font, disfont, prbfont, colormap, visual, backbuf);
    fprintf(stderr, "フォントの読み込みに失敗。\n");
    exit(1);
  }

  prbfont = XftFontOpenName(display, screen, "Noto Sans CJK-24");
  if (!prbfont) {
    cleanup(display, window, gc, &color, &btncolor, font, disfont, prbfont, colormap, visual, backbuf);
    fprintf(stderr, "問題フォントの読み込みに失敗。\n");
    exit(1);
  }

  disfont = XftFontOpenName(display, screen, "Noto Sans CJK-72");
  if (!disfont) {
    cleanup(display, window, gc, &color, &btncolor, font, disfont, prbfont, colormap, visual, backbuf);
    fprintf(stderr, "解決フォントの読み込みに失敗。\n");
    exit(1);
  }

  if (!XftColorAllocName(display, visual, colormap, "#232020", &color)) {
    cleanup(display, window, gc, &color, &btncolor, font, disfont, prbfont, colormap, visual, backbuf);
    fprintf(stderr, "色の役割に失敗。\n");
    exit(1);
  }

  XMapWindow(display, window);
  {
    XWindowAttributes attr;
    XGetWindowAttributes(display, window, &attr);
    XEvent fake = { .type = Expose };
    fake.xexpose.window = window;
    fake.xexpose.width = attr.width;
    fake.xexpose.height = attr.height;
    control_expose(display, window, gc, &event, &color, font, disfont, prbfont);
  }

  while (isrunning) {
    XNextEvent(display, &event);

    switch (event.type) {
      case Expose:
      case ConfigureNotify:
        XClearWindow(display, window);
        control_expose(display, window, gc, &event, &color, font, disfont, prbfont);
        break;
      case ButtonPress:
        if (event.xbutton.button == Button1) {
          handle_button_press(display, gc, event.xbutton.x, event.xbutton.y, &color, font);
          break;
        }
      case ButtonRelease:
        if (event.xbutton.button == Button1) {
          handle_button_release(display, window, gc, event.xbutton.x, event.xbutton.y, &color, font, disfont, prbfont);
          break;
        }
      case KeyPress:
        handle_key_press(display, window, gc, &event, &color, font, disfont, prbfont);
        break;
      // case MotionNotify:
      //   handle_mouse_hover(display, window, gc, &event, &color, font);
      //   break;
      case ClientMessage:
        // WM_DELETE_WINDOW
        break;
    }
  }

  cleanup(display, window, gc, &color, &btncolor, font, disfont, prbfont, colormap, visual, backbuf);

  return 0;
}
