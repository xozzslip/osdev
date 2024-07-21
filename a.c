#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    Display *display;
    Window window;
    XEvent event;
    int s;

    /* Open connection to the X server */
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    s = DefaultScreen(display);

    /* Create a window */
    window = XCreateSimpleWindow(display, RootWindow(display, s), 10, 10, 200, 200, 1,
                                 BlackPixel(display, s), WhitePixel(display, s));

    /* Select kind of events we are interested in */
    XSelectInput(display, window, KeyPressMask);

    /* Map (show) the window */
    XMapWindow(display, window);
    write(1, "-------xui\n", 11);


    int events = XPending(display);

    write(1, "-------jui\n", 11);

    /* close connection to server */
    XCloseDisplay(display);

    return 0;
}
