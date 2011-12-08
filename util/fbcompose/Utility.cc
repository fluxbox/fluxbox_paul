/** Utility.cc file for the fluxbox compositor. */

// Copyright (c) 2011 Gediminas Liktaras (gliktaras at gmail dot com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include "Utility.hh"

#include "BaseScreen.hh"


//--- FUNCTIONS ----------------------------------------------------------------

// Creates a new pixmap, filled with the specified color.
Pixmap FbCompositor::createSolidPixmap(const BaseScreen &screen, int width,
                                       int height, unsigned long color) {
    Display *display = (Display*)(screen.display());

    Pixmap pixmap = XCreatePixmap(display, screen.rootWindow().window(), width, height, 32);

    GC gc = XCreateGC(display, pixmap, 0, NULL);
    XSetForeground(display, gc, color);
    XFillRectangle(display, pixmap, gc, 0, 0, width, height);
    XFreeGC(display, gc);

    return pixmap;
}

// Computes the highest power of two that equals or is less than the given value.
// Assumption: signed 32 bit integers.
int FbCompositor::largestSmallerPowerOf2(int value) {
    int power = 30;
    while (!(value & (1 << power)) && (power >= 0)) {
        power--;
    }

    if (power < 0) {
        return 0;
    } else {
        return (1 << power);
    }
}

// Returns the location of the mouse pointer.
void FbCompositor::mousePointerLocation(const BaseScreen &screen, int &root_x_return, int &root_y_return) {
    static Window root_win, child_win;
    static int child_x, child_y;
    static unsigned int mask;

    XQueryPointer((Display*)(screen.display()), screen.rootWindow().window(),
                  &root_win, &child_win, &root_x_return, &root_y_return, &child_x, &child_y, &mask);
}
