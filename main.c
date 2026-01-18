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
#include <stdio.h>

#include <X11/Xatom.h>
#include "control.h"

const char *sofname = "ucalc";
const char *version = "0.0.0";
const char *disname = "Unix Calc";

int main() {
#if defined(__OpenBSD__)
  SuwaWindow window = suwaui_create_window(
      500, 400, 382, 534,
      disname, version, "unixcalc", "UnixCalc",
      "Noto Sans CJK-8", "#232320",
      0, 1, 0);
  SuwaLabel resLabel = suwaui_add_label(&window, 20, 180, 0, 0, "0",
      "Noto Sans CJK-48", "#f1ed25", 1);
  SuwaLabel problemLabel = suwaui_add_label(&window, 20, 80, 0, 0, "",
      "Noto Sans CJK-12", "#b8b515", 1);
#elif defined(__FreeBSD__)
  SuwaWindow window = suwaui_create_window(
      20, 40, 382, 534,
      disname, version, "unixcalc", "UnixCalc",
      "Noto Sans CJK-12", "#232020",
      0, 1, 0);
  SuwaLabel resLabel = suwaui_add_label(&window, 20, 180, 0, 0, "0",
      "Noto Sans CJK-72", "#ee4030", 1);
  SuwaLabel problemLabel = suwaui_add_label(&window, 20, 80, 0, 0, "",
      "Noto Sans CJK-24", "#b61729", 1);
#endif

  CtrlLabels lbl = {
    .res = &resLabel,
    .problem = &problemLabel
  };

  XMapWindow(window.display, window.xwindow);
  {
    XWindowAttributes attr;
    XGetWindowAttributes(window.display, window.xwindow, &attr);
    XEvent fake = { .type = Expose };
    fake.xexpose.window = window.xwindow;
    fake.xexpose.width = attr.width;
    fake.xexpose.height = attr.height;
    control_expose(&window, &lbl);
  }

  while (window.isrunning) {
    XNextEvent(window.display, &window.event);

    switch (window.event.type) {
      case Expose:
      case ConfigureNotify:
        XClearWindow(window.display, window.xwindow);
        control_expose(&window, &lbl);
        break;
      case ButtonPress:
        if (window.event.xbutton.button == Button1) {
          int x = window.event.xbutton.x;
          int y = window.event.xbutton.y;
          handle_button_press(&window, x, y);
          break;
        }
      case ButtonRelease:
        if (window.event.xbutton.button == Button1) {
          int x = window.event.xbutton.x;
          int y = window.event.xbutton.y;
          handle_button_release(&window, &lbl, x, y);
          break;
        }
      case KeyPress:
        handle_key_press(&window, &lbl);
        break;
      // case MotionNotify:
      //   handle_mouse_hover(&window);
      //   break;
      case ClientMessage:
        // WM_DELETE_WINDOW
        break;
    }
  }

  suwaui_del_label(&window, &problemLabel);
  suwaui_del_label(&window, &resLabel);
  suwaui_destroy_window(&window);

  return 0;
}
