
#ifndef DISPLAY_STRUCT_HEADER
#define DISPLAY_STRUCT_HEADER

#include <X11/Xlib.h>

typedef void (*Callback)(void *cbdata);
typedef struct Button Button;
typedef struct exitInfo ExitInfo;
typedef struct startInfo StartInfo;
typedef struct stopInfo StopInfo;
typedef struct selectInfo SelectInfo;
typedef struct fileInfo FileInfo;
struct exitInfo {
	Display *dpy;
	XFontStruct *font;
};
struct startInfo {
	Display *dpy;
	XFontStruct *font;
};
struct stopInfo {
	Display *dpy;
	XFontStruct *font;
};
struct selectInfo {
	Display *dpy;
	XFontStruct *font;
	XrmDatabase db;
	XContext ctxt;
};
struct fileInfo {
	Display *dpy;
	XFontStruct *font;
};
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