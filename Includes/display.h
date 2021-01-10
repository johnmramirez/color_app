#ifndef DISPLAY_HEADER
#define DISPLAY_HEADER

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>

#include "../Includes/display_structs.h"

int XChar2bLen(XChar2b *);
int utf8toXChar2b(XChar2b *, int, const char *, int);
unsigned long getColour(Display *,  XrmDatabase, char *,
			char *, char *);
XFontStruct *getFont(Display *, XrmDatabase, char *,
		char *, char *);
void exitButton(void *);
void startButton(void *);
void stopButton(void *);
void createButton(Display *, Window, char *, XFontStruct *,
		int, int, int, int,
		unsigned long, unsigned long, unsigned long,
			XContext, Callback, void *);
XContext setup(Display *, int, char **);
void buttonExpose(Button *, XEvent *);
void buttonConfigure(Button *, XEvent *);
void buttonEnter(Button *, XEvent *);
void buttonLeave(Button *, XEvent *);

#endif