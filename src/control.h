#pragma once

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

void control_expose(Display *dpy, Window wnd, GC gc, XEvent *event, XftColor *color, XftFont *f, XftFont *df, XftFont *pf);
void handle_button_press(Display *dpy, GC gc, int mx, int my, XftColor *col, XftFont *f);
void handle_button_release(Display *dpy, Window wnd, GC gc, int mx, int my, XftColor *col, XftFont *f, XftFont *df, XftFont *pf);
void handle_key_press(Display *dpy, Window wnd, GC gc, XEvent *event, XftColor *col, XftFont *f, XftFont *df, XftFont *pf);
void handle_mouse_hover(Display *dpy, Window wnd, GC gc, XEvent *event, XftColor *col, XftFont *f);
