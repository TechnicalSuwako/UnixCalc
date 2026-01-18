#pragma once

#include <suwaui.h>

typedef struct {
  SuwaLabel *res;
  SuwaLabel *problem;
} CtrlLabels;

void control_expose(SuwaWindow *window, CtrlLabels *labels);
void handle_button_press(SuwaWindow *window, int mx, int my);
void handle_button_release(SuwaWindow *window, CtrlLabels *labels, int mx, int my);
void handle_key_press(SuwaWindow *window, CtrlLabels *labels);
// void handle_mouse_hover(SuwaWindow *window);
