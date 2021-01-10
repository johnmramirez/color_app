
#ifndef DISPLAY_STRUCT_HEADER
#define DISPLAY_STRUCT_HEADER

#include <X11/Xlib.h>

typedef void (*Callback)(void *cbdata);
typedef struct Button Button;
struct Button {
	XChar2b * text;
	int text_width;
	int font_ascent;
	int width, height;
	unsigned long border, background, foreground;
	void *cbdata;
	Callback buttonRelease;
};

#endif