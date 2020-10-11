#include <Kernel/Asserts.hpp>
#include <Kernel/Types.hpp>
#include <Logger/Logger.hpp>
#include <UnixApp.hpp>

logging::ChannelID PLAT_UNIX_ID = LOG_GET_ID();

#include <X11/Xlib.h>  // Every Xlib program must include this
#include <unistd.h>
#define NIL (0)  // A name for the void pointer

void launchTest()
{
    // Open the display

    Display* dpy = XOpenDisplay(NIL);
    mAssert(dpy != NULL);

    // Get some colors

    int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
    int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

    // Create the window

    Window w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 200, 100,
                                   0, blackColor, blackColor);

    // We want to get MapNotify events

    XSelectInput(dpy, w, StructureNotifyMask);

    // "Map" the window (that is, make it appear on the screen)

    XMapWindow(dpy, w);

    // Create a "Graphics Context"

    GC gc = XCreateGC(dpy, w, 0, NIL);

    // Tell the GC we draw using the white color

    XSetForeground(dpy, gc, whiteColor);

    // Wait for the MapNotify event

    for (;;)
    {
        XEvent e;
        XNextEvent(dpy, &e);
        if (e.type == MapNotify)
            break;
    }

    // Draw the line
    const char* text = "Hello World";
    XDrawString(dpy, w, gc, 10, 10, text, 11);
    // XDrawText(dpy, w, gc, 10, 10, XTextItem *items, 1);
    // XDrawLine(dpy, w, gc, 10, 60, 180, 20);

    // Send the "DrawLine" request to the server

    XFlush(dpy);

    // Wait for 10 seconds

    sleep(10);
}