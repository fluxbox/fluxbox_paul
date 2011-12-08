/** Utility.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_UTILITY_HH
#define FBCOMPOSITOR_UTILITY_HH

#include <X11/Xlib.h>

#include <algorithm>


namespace FbCompositor {

    class BaseScreen;


    //--- MACROS ---------------------------------------------------------------

    /** Suppress compiler warning for unused parameters. */
    #define MARK_PARAMETER_UNUSED(x) (void)(x)


    //--- FUNCTIONS ------------------------------------------------------------

    /** Creates a new pixmap, filled with the specified color. */
    Pixmap createSolidPixmap(const BaseScreen &screen, int width, int height, unsigned long color = 0x00000000);

    /** Computes the highest power of two that equals or is less than the given value. */
    int largestSmallerPowerOf2(int value);

    /** \returns the location of the mouse pointer. */
    void mousePointerLocation(const BaseScreen &screen, int &root_x_return, int &root_y_return);

}

#endif  // FBCOMPOSITOR_UTILITY_HH
