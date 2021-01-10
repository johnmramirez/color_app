#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Includes/display_structs.h"
#include "../Includes/display.h"

#define DefGC(dpy) DefaultGC(dpy, DefaultScreen(dpy))



int XChar2bLen(XChar2b *string){
	int j = 0;
	for(j = 0; string[j].byte1 || string[j].byte2; j ++ )
			;
	return j;
}

int utf8toXChar2b(XChar2b *output_r, int outsize, const char *input, int inlen){
	int j, k;
	for(j =0, k=0; j < inlen && k < outsize; j ++){
		unsigned char c = input[j];
		if (c < 128)  {
			output_r[k].byte1 = 0;
			output_r[k].byte2 = c; 
			k++;
		} else if (c < 0xC0) {
			/* we're inside a character we don't know  */
			continue;
		} else switch(c&0xF0){
		case 0xC0: case 0xD0: /* two bytes 5+6 = 11 bits */
			if (inlen < j+1){ return k; }
			output_r[k].byte1 = (c&0x1C) >> 2;
			j++;
			output_r[k].byte2 = ((c&0x3) << 6) + (input[j]&0x3F);
			k++;
			break;
		case 0xE0: /* three bytes 4+6+6 = 16 bits */ 
			if (inlen < j+2){ return k; }
			j++;
			output_r[k].byte1 = ((c&0xF) << 4) + ((input[j]&0x3C) >> 2);
			c = input[j];
			j++;
			output_r[k].byte2 = ((c&0x3) << 6) + (input[j]&0x3F);
			k++;
			break;
		case 0xFF:
			/* the character uses more than 16 bits */
			continue;
		}
	}
	output_r[k].byte2 = 0;
	output_r[k].byte1 = 0;
	return k;
}

unsigned long getColour(Display *dpy,  XrmDatabase db, char *name,
			char *cl, char *def){
	XrmValue v;
	XColor col1, col2;
	Colormap cmap = DefaultColormap(dpy, DefaultScreen(dpy));
	char * type;

	if (XrmGetResource(db, name, cl, &type, &v)
			&& XAllocNamedColor(dpy, cmap, v.addr, &col1, &col2)) {
	} else {
		XAllocNamedColor(dpy, cmap, def, &col1, &col2);
	}
	return col2.pixel;
}

XFontStruct *getFont(Display *dpy, XrmDatabase db, char *name,
		char *cl, char *def){
	XrmValue v;
	char * type;
	XFontStruct *font = NULL;

	if (XrmGetResource(db, name, cl, &type, &v)){
		if (v.addr)
			font = XLoadQueryFont(dpy, v.addr);
	}
	if (!font) {
		if (v.addr)
		fprintf(stderr, "unable to load preferred font: %s using fixed\n", v.addr);
		else 
		fprintf(stderr, "couldn't figure out preferred font\n");
		font = XLoadQueryFont(dpy, def);
	}
	return font;
}

void createButton(Display *dpy, Window parent, char *text, XFontStruct *font,
		int x, int y, int width, int height,
		unsigned long foreground, unsigned long background, unsigned long border,
			XContext ctxt, Callback callback, void *cbdata){
	Button *button;
	Window win;
	int strlength = strlen(text);

	win = XCreateSimpleWindow(dpy, parent, x, y, width, height,
		2, border, background); /* borderwidth, border and background colour */
	if (!win) {
		fprintf(stderr, "unable to create a subwindow\n");
		exit(31);
	}

	button = calloc(sizeof(*button), 1);
	if (!button){
		fprintf(stderr, "unable to allocate any space, dieing\n");
		exit(32);
	}

	button->font_ascent = font->ascent;

	button->text = malloc(sizeof(*button->text) * (strlength+1));
	if (!button->text){
		fprintf(stderr, "unable to allocate any string space, dieing\n");
		exit(32);
	}
	strlength = utf8toXChar2b(button->text, strlength, text, strlength);
	button->text_width = XTextWidth16(font, button->text, strlength);
	button->buttonRelease = callback;
	button->cbdata = cbdata;
	button->width = width;
	button->height = height;
	button->background = background;
	button->foreground = foreground;
	button->border = border;

	XSelectInput(dpy, win,
		ButtonPressMask|ButtonReleaseMask|StructureNotifyMask|ExposureMask);

	XSaveContext(dpy, win, ctxt, (XPointer)button);
	XMapWindow(dpy, win);
}

void createFileSelect(Display *dpy, Window win, XrmDatabase db, XContext ctxt, XFontStruct * font){
	
	Button *filewindow;

	filewindow = calloc(sizeof(*filewindow), 1);
	if (!filewindow){
		fprintf(stderr, "unable to allocate any space, dieing\n");
		exit(32);
	}

	filewindow->background = getColour(dpy,  db, "xtut9.background", "xtut9.BackGround", "Black");
	filewindow->border = getColour(dpy,  db, "xtut9.border", "xtut9.Border", "White");
	filewindow->foreground = getColour(dpy,  db, "xtut9.foreground", "xtut9.ForeGround", "White");

	filewindow->width = 400;
	filewindow->height = 100;

	win = XCreateSimpleWindow(dpy, win, 0, 0, filewindow->width, filewindow->height,
		2, filewindow->border, filewindow->background); /* borderwidth, border and background colour */
	if (!win) {
		fprintf(stderr, "unable to create a subwindow\n");
		exit(31);
	}
	Xutf8SetWMProperties(dpy, win, "Select File", "xtut9", NULL, 0,
		NULL, NULL, NULL);

	FileInfo *fileInfo;
	fileInfo = malloc(sizeof(*fileInfo));
	fileInfo->dpy = dpy;
	fileInfo->font = font;
	createButton(dpy, win, "Read", font, /*display text font */
		filewindow->width/2, filewindow->height, 80, (font->ascent+font->descent)*2,/*xywh*/
			/* colours */
		filewindow->foreground, filewindow->background, filewindow->border,
		ctxt, fileButton, fileInfo);			/* context & callback info */

	XSelectInput(dpy, win, StructureNotifyMask|ExposureMask);
    XMapWindow(dpy, win);
	XSaveContext(dpy, win, ctxt, (XPointer)filewindow);
}

void exitButton(void *cbdata){
	ExitInfo *ei = (ExitInfo*)cbdata;
	XFreeFont(ei->dpy, ei->font);
	XCloseDisplay(ei->dpy);
	exit(0);
}

void startButton(void *cbdata){

}

void stopButton(void *cbdata){

}

void selectButton(void *cbdata){
	SelectInfo *si = (SelectInfo*)cbdata;
	createFileSelect(si->dpy, DefaultRootWindow(si->dpy), si->db, si->ctxt, si->font);
}

void fileButton(void *cbdata){

}

XContext setup(Display * dpy, int argc, char ** argv){
	static XrmOptionDescRec xrmTable[] = {
		{"-bg", "*background", XrmoptionSepArg, NULL},
		{"-fg", "*foreground", XrmoptionSepArg, NULL},
		{"-bc", "*bordercolour", XrmoptionSepArg, NULL},
		{"-font", "*font", XrmoptionSepArg, NULL},
	};
	Button *mainwindow;
	Window win;
	XGCValues values;

	XFontStruct * font;
	XrmDatabase db;

	XContext ctxt;

	ctxt = XUniqueContext();

	mainwindow = calloc(sizeof(*mainwindow), 1);

	XrmInitialize();
	db = XrmGetDatabase(dpy);
	XrmParseCommand(&db, xrmTable, sizeof(xrmTable)/sizeof(xrmTable[0]),
		"xtut9", &argc, argv);

	font = getFont(dpy, db, "xtut9.font", "xtut9.Font", "fixed");
	mainwindow->background = getColour(dpy,  db, "xtut9.background", "xtut9.BackGround", "Black");
	mainwindow->border = getColour(dpy,  db, "xtut9.border", "xtut9.Border", "White");
	mainwindow->foreground = values.foreground = getColour(dpy,  db, "xtut9.foreground", "xtut9.ForeGround", "White");


	mainwindow->width = 400;
	mainwindow->height = 400;

	win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), /* display, parent */
		0,0, /* x, y: the window manager will place the window elsewhere */
		mainwindow->width, mainwindow->height, /* width, height */
		2, mainwindow->border, /* border width & colour, unless you have a window manager */
		mainwindow->background); /* background colour */

	Xutf8SetWMProperties(dpy, win, "XTut9", "xtut9", argv, argc,
		NULL, NULL, NULL);

	/* make the default pen what we want */
	values.line_width = 1;
	values.line_style = LineSolid;
	values.font = font->fid;

	XChangeGC(dpy, DefGC(dpy),
		GCForeground|GCLineWidth|GCLineStyle|GCFont,&values);


	/* Button Start */
	SelectInfo *selectInfo;
	selectInfo = malloc(sizeof(*selectInfo));
	selectInfo->dpy = dpy;
	selectInfo->font = font;
	selectInfo->ctxt = ctxt;
	selectInfo->db = db;
	createButton(dpy, win, "Select", font, /*display text font */
		mainwindow->width/2-200, mainwindow->height - (font->ascent+font->descent)*2 - 4, 80, (font->ascent+font->descent)*2,/*xywh*/
			/* colours */
		mainwindow->foreground, mainwindow->background, mainwindow->border,
		ctxt, selectButton, selectInfo);			/* context & callback info */

	StartInfo *startInfo;
	startInfo = malloc(sizeof(*startInfo));
	startInfo->dpy = dpy;
	startInfo->font = font;
	createButton(dpy, win, "Start", font, /*display text font */
		mainwindow->width/2-120, mainwindow->height - (font->ascent+font->descent)*2 - 4, 80, (font->ascent+font->descent)*2,/*xywh*/
			/* colours */
		mainwindow->foreground, mainwindow->background, mainwindow->border,
		ctxt, startButton, startInfo);			/* context & callback info */

	StopInfo *stopInfo;
	stopInfo = malloc(sizeof(*stopInfo));
	stopInfo->dpy = dpy;
	stopInfo->font = font;
	createButton(dpy, win, "Stop", font, /*display text font */
		mainwindow->width/2-40, mainwindow->height - (font->ascent+font->descent)*2 - 4, 80, (font->ascent+font->descent)*2,/*xywh*/
			/* colours */
		mainwindow->foreground, mainwindow->background, mainwindow->border,
		ctxt, stopButton, stopInfo);			/* context & callback info */

	ExitInfo *exitInfo;
	exitInfo = malloc(sizeof(*exitInfo));
	exitInfo->dpy = dpy;
	exitInfo->font = font;
	createButton(dpy, win, "Exit", font, /*display text font */
		mainwindow->width/2+40, mainwindow->height - (font->ascent+font->descent)*2 - 4, 80, (font->ascent+font->descent)*2,/*xywh*/
			/* colours */
		mainwindow->foreground, mainwindow->background, mainwindow->border,
		ctxt, exitButton, exitInfo);			/* context & callback info */
	
	/* Button end */

	/* tell the display server what kind of events we would like to see */
	XSelectInput(dpy, win, StructureNotifyMask|ExposureMask);
	/* okay, put the window on the screen, please */
	XMapWindow(dpy, win);

	/* save the useful information about the window */
	XSaveContext(dpy, win, ctxt, (XPointer)mainwindow);

	return ctxt;
}

void buttonExpose(Button *button, XEvent *ev) {
	int textx, texty, len;
	if (!button) return;
	if (button->text){
		len = XChar2bLen(button->text);
   		textx = (button->width - button->text_width)/2;
   		texty = (button->height + button->font_ascent)/2;
   		XDrawString16(ev->xany.display, ev->xany.window, DefGC(ev->xany.display), textx, texty,
			button->text, len);
	}
}
void buttonConfigure(Button *button, XEvent *ev){
	if (!button) return;
	if (button->width != ev->xconfigure.width
			|| button->height != ev->xconfigure.height) {
		button->width = ev->xconfigure.width;
		button->height = ev->xconfigure.height;
		XClearWindow(ev->xany.display, ev->xany.window);
	}
}

void buttonEnter(Button *button, XEvent *ev) {
	XSetWindowAttributes attrs;
	if(!button) return;
	attrs.background_pixel = button->border;
	attrs.border_pixel = button->background;
	XChangeWindowAttributes(ev->xany.display, ev->xany.window,
			CWBackPixel|CWBorderPixel, &attrs);
	XClearArea(ev->xany.display, ev->xany.window, 0, 0, button->width, button->height, True);
}
void buttonLeave(Button *button, XEvent *ev) {
	XSetWindowAttributes attrs;
	if(!button) return;
	attrs.background_pixel = button->background;
	attrs.border_pixel = button->border;
	XChangeWindowAttributes(ev->xany.display, ev->xany.window,
			CWBackPixel|CWBorderPixel, &attrs);
	XClearArea(ev->xany.display, ev->xany.window, 0, 0, button->width, button->height, True);
}