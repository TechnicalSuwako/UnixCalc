#pragma once

typedef struct {
  int x, y, width, height;
  const char *label;
  int pressed; // 0 = 普通、1 = 押している
} SuwaButton;
