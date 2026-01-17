#include <stdio.h>

#include <X11/Xatom.h>
#include "src/utils.h"
#include "src/control.h"

const char *sofname = "ucalc";
const char *version = "0.0.0";
const char *disname = "Unix Calc";

int main() {
  UiSystem ui;
  ui.isrunning = 1;
  ui.window.width = 382;
  ui.window.height = 534;
  XEvent event;
  int screen;
  XGCValues values;

  ui.display = XOpenDisplay(NULL);
  if (ui.display == NULL) {
    fprintf(stderr, "画面を開けられません。\n");
    exit(1);
  }

  screen = DefaultScreen(ui.display);

  int sw = DisplayWidth(ui.display, screen);
  int sh = DisplayHeight(ui.display, screen);
  ui.window.x = (sw - ui.window.width) / 3;
  ui.window.y = (sh - ui.window.height) / 2;

  ui.xwindow = XCreateSimpleWindow(ui.display,
      RootWindow(ui.display, screen),
      ui.window.x, ui.window.y,
      ui.window.width, ui.window.height,
      1, BTCOL, BGCOL);
  if (!ui.xwindow) {
    cleanup(&ui);
    fprintf(stderr, "ウィンドウを作成に失敗。\n");
    exit(1);
  }

  ui.backbuf = XCreatePixmap(ui.display, ui.xwindow,
      ui.window.width, ui.window.height,
      DefaultDepth(ui.display, screen));
  ui.target = ui.backbuf;

  Atom net_wm_window_type = XInternAtom(ui.display, "_NET_WM_WINDOW_TYPE", False);
  Atom dialog = XInternAtom(ui.display, "_NET_WM_WINDOW_TYPE_DIALOG", False);

  XChangeProperty(ui.display, ui.xwindow, net_wm_window_type, XA_ATOM, 32,
      PropModeReplace, (unsigned char *)&dialog, 1);

  XStoreName(ui.display, ui.xwindow, disname);
  Atom net_wm_name = XInternAtom(ui.display, "_NET_WM_NAME", False);
  char displayname[16];
  snprintf(displayname, 16, "%s %s", disname, version);
  XChangeProperty(ui.display, ui.xwindow, net_wm_name,
      XInternAtom(ui.display, "UTF8_STRING", False), 8,
      PropModeReplace, (unsigned char *)displayname, strlen(displayname));

  XClassHint *classHint = XAllocClassHint();
  if (classHint) {
    classHint->res_name = "unixcalc";
    classHint->res_class = "UnixCalc";
    XSetClassHint(ui.display, ui.xwindow, classHint);
    XFree(classHint);
  }

  XSetWindowBackground(ui.display, ui.xwindow, BGCOL);

  XSelectInput(ui.display, ui.xwindow,
      ExposureMask
    | ButtonPressMask
    | ButtonReleaseMask
    | KeyPressMask
    // | PointerMotionMask
    // | ButtonMotionMask
    // | StructureNotifyMask
  );

  ui.gc = XCreateGC(ui.display, ui.xwindow, 0, &values);
  if (!ui.gc) {
    cleanup(&ui);
    fprintf(stderr, "GCを作成に失敗。\n");
    exit(1);
  }

  ui.visual = *DefaultVisual(ui.display, screen);

  ui.colormap = XCreateColormap(ui.display, ui.xwindow, &ui.visual, AllocNone);
  if (ui.colormap == None) {
    cleanup(&ui);
    fprintf(stderr, "カラーマップを作成に失敗。\n");
    exit(1);
  }

  XWindowAttributes attr;
  XGetWindowAttributes(ui.display, ui.xwindow, &attr);
  int w = attr.width;

  {
    ui.resLabel.font = XftFontOpenName(ui.display, screen, "Noto Sans CJK-72");
    if (!ui.resLabel.font) {
      cleanup(&ui);
      fprintf(stderr, "解決フォントの読み込みに失敗。\n");
      exit(1);
    }

    ui.resLabel.text[0] = '0';
    ui.resLabel.text[1] = '\0';

    const FcChar8 *text = (const FcChar8 *)ui.resLabel.text;
    int len = strlen((const char *)text);

    XGlyphInfo extents;
    XftTextExtentsUtf8(ui.display, ui.resLabel.font, text, len, &extents);

    ui.resLabel.x = w - 20 - extents.xOff;
    ui.resLabel.y = 180;
    if (!XftColorAllocName(ui.display,
        DefaultVisual(ui.display, DefaultScreen(ui.display)),
        DefaultColormap(ui.display, DefaultScreen(ui.display)),
        "#ee4030", &ui.resLabel.fg_color)) {
      cleanup(&ui);
      fprintf(stderr, "色の役割に失敗。\n");
      exit(1);
    }
  }

  {
    ui.problemLabel.font = XftFontOpenName(ui.display, screen, "Noto Sans CJK-24");
    if (!ui.problemLabel.font) {
      cleanup(&ui);
      fprintf(stderr, "問題フォントの読み込みに失敗。\n");
      exit(1);
    }

    ui.problemLabel.text[0] = '\0';

    const FcChar8 *text = (const FcChar8 *)ui.problemLabel.text;
    int len = strlen((const char *)text);

    XGlyphInfo extents;
    XftTextExtentsUtf8(ui.display, ui.problemLabel.font, text, len, &extents);

    ui.problemLabel.x = w - 20 - extents.xOff;
    ui.problemLabel.y = 80;
    if (!XftColorAllocName(ui.display,
        DefaultVisual(ui.display, DefaultScreen(ui.display)),
        DefaultColormap(ui.display, DefaultScreen(ui.display)),
        "#b61729", &ui.problemLabel.fg_color)) {
      cleanup(&ui);
      fprintf(stderr, "色の役割に失敗。\n");
      exit(1);
    }
  }

  ui.font = XftFontOpenName(ui.display, screen, "Noto Sans CJK-12");
  if (!ui.font) {
    cleanup(&ui);
    fprintf(stderr, "フォントの読み込みに失敗。\n");
    exit(1);
  }

  if (!XftColorAllocName(ui.display, &ui.visual, ui.colormap, "#232020", &ui.color)) {
    cleanup(&ui);
    fprintf(stderr, "色の役割に失敗。\n");
    exit(1);
  }

  XMapWindow(ui.display, ui.xwindow);
  {
    XWindowAttributes attr;
    XGetWindowAttributes(ui.display, ui.xwindow, &attr);
    XEvent fake = { .type = Expose };
    fake.xexpose.window = ui.xwindow;
    fake.xexpose.width = attr.width;
    fake.xexpose.height = attr.height;
    control_expose(&ui, &event);
  }

  while (ui.isrunning) {
    XNextEvent(ui.display, &event);

    switch (event.type) {
      case Expose:
      case ConfigureNotify:
        XClearWindow(ui.display, ui.xwindow);
        control_expose(&ui, &event);
        break;
      case ButtonPress:
        if (event.xbutton.button == Button1) {
          handle_button_press(&ui, event.xbutton.x, event.xbutton.y);
          break;
        }
      case ButtonRelease:
        if (event.xbutton.button == Button1) {
          handle_button_release(&ui, event.xbutton.x, event.xbutton.y);
          break;
        }
      case KeyPress:
        handle_key_press(&ui, &event);
        break;
      // case MotionNotify:
      //   handle_mouse_hover(&ui, &event);
      //   break;
      case ClientMessage:
        // WM_DELETE_WINDOW
        break;
    }
  }

  cleanup(&ui);

  return 0;
}
