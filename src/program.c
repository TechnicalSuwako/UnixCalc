#include "program.h"

int window_width = 382;
int window_height = 534;
int isrunning = 1;
char displayprb[128] = "";
char curinput[256] = {0};
char displaytxt[64] = "0";
int input_pos = 0;
Pixmap backbuf = None;
SuwaButton *hovered_btn = NULL;
SuwaButton *pressed_btn = NULL;
