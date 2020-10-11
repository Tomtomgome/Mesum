#include <Kernel/Asserts.hpp>
#include <Kernel/Types.hpp>
#include <Logger/Logger.hpp>
#include <UnixApp.hpp>


#include <X11/Xlib.h>  // Every Xlib program must include this
#include <X11/keysym.h>  // Every Xlib program must include this
#include <unistd.h>

#include <chrono>
#include <thread>

#define NIL (0)  // A name for the void pointer

namespace m
{
    logging::ChannelID PLAT_UNIX_ID = LOG_GET_ID();

    void launchTest()
    {
        // Open the display

        Display* dpy = XOpenDisplay(NIL);
        mAssert(dpy != NULL);

        // Get some colors

        int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
        int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

        // Create the window

        Window w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 600, 400,
                                    0, blackColor, blackColor);

        // We want to get MapNotify events

        XSelectInput(dpy, w, StructureNotifyMask | KeyPressMask | KeyReleaseMask);

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

        m::Float x            = 0.0f;
        m::Float y            = 0.0f;
        bool     windowClosed = false;
        bool     up           = false;
        bool     down         = false;
        bool     left         = false;
        bool     right        = false;
        while (!windowClosed)
        {
            auto start = std::chrono::high_resolution_clock::now();

            m::Int queueLength = XQLength(dpy);
            for (m::Int i = 0; i < queueLength; ++i)
            {
                XEvent e;
                XNextEvent(dpy, &e);
                if (e.type == DestroyNotify)
                {
                    windowClosed = true;
                }

                if (e.type == KeyPress || e.type == KeyRelease)
                {
                    switch (XLookupKeysym(&e.xkey, 0))
                    {
                        case XK_Up:
                        {
                            up = e.type == KeyPress;
                            break;
                        }
                        case XK_Down:
                        {
                            down = e.type == KeyPress;
                            break;
                        }
                        case XK_Left:
                        {
                            left = e.type == KeyPress;
                            break;
                        }
                        case XK_Right:
                        {
                            right = e.type == KeyPress;
                            break;
                        }
                    }
                }
            }

            float speed = 0.016 * 100;
            if (up)
            {
                y -= speed;
            }
            if (down)
            {
                y += speed;
            }
            if (left)
            {
                x -= speed;
            }
            if (right)
            {
                x += speed;
            }

            XClearWindow(dpy, w);
            XDrawRectangle(dpy, w, gc, x, y, 5, 5);
            XFlush(dpy);

            auto      end = std::chrono::high_resolution_clock::now();
            long long timming =
                std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                    .count();
            if (timming < 16)
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(16 - timming));
            }
        }
    }
};