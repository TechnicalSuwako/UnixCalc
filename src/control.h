#pragma once

#include "ui.h"

void control_expose(UiSystem *ui, XEvent *event);
void handle_button_press(UiSystem *ui, int mx, int my);
void handle_button_release(UiSystem *ui, int mx, int my);
void handle_key_press(UiSystem *ui, XEvent *event);
void handle_mouse_hover(UiSystem *ui, XEvent *event);
