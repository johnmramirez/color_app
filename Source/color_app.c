#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <stdio.h>
#include <stdlib.h>

#include "../Includes/display_structs.h"
#include "../Includes/display.h"

int main_loop(Display *dpy, XContext context){
	XEvent ev;

	/* as each event that we asked about occurs, we respond. */
	while(1){
		Button *button = NULL;
		XNextEvent(dpy, &ev);
		XFindContext(ev.xany.display, ev.xany.window, context, (XPointer*)&button);
		switch(ev.type){
		/* configure notify will only be sent to the main window */
		case ConfigureNotify:
			if (button)
				buttonConfigure(button, &ev);
			break;
		/* expose will be sent to both the button and the main window */
		case Expose:
			if (ev.xexpose.count > 0) break;
			if (button)
				buttonExpose(button, &ev);
			break;

		/* these three events will only be sent to the button */
		// case EnterNotify:
		// 	if (button)
		// 		buttonEnter(button, &ev);
		// 	break;
		// case LeaveNotify:
		// 	if (button)
		// 		buttonLeave(button, &ev);
		// 	break;
		case ButtonRelease:
			if (button && button->buttonRelease)
				button->buttonRelease(button->cbdata);
			break;
		}
	}
}

int main(int argc, char ** argv){
	Display *dpy;
	XContext ctxt;

	/* First connect to the display server */
	dpy = XOpenDisplay(NULL);
	if (!dpy) {fprintf(stderr, "unable to connect to display\n");return 7;}
	ctxt = setup(dpy, argc, argv);
	return main_loop(dpy, ctxt);
}
