/** OpenGLWindow.cc file for the fluxbox compositor. */

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


#include "OpenGLWindow.hh"

#include "BaseScreen.hh"
#include "Logging.hh"
#include "OpenGLScreen.hh"
#include "OpenGLUtility.hh"
#include "Utility.hh"

#include <X11/Xutil.h>

#include <algorithm>
#include <iostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLWindow::OpenGLWindow(const OpenGLScreen &screen, Window window_xid) :
    BaseCompWindow(static_cast<const BaseScreen&>(screen), window_xid, false) {

    m_content_tex_partition = new OpenGL2DTexturePartition(screen, true);
    m_shape_tex_partition = new OpenGL2DTexturePartition(screen, false);

    updateWindowPos();
}

// Destructor.
OpenGLWindow::~OpenGLWindow() { }


//--- ACCESSORS ----------------------------------------------------------------

// Returns the window's screen, cast into the correct class.
const OpenGLScreen &OpenGLWindow::openGLScreen() const {
    static const OpenGLScreen &s = dynamic_cast<const OpenGLScreen&>(screen());
    return s;
}


//--- WINDOW UPDATE FUNCTIONS --------------------------------------------------

// Updates the window's contents.
void OpenGLWindow::updateContents() {
    updateContentPixmap();
    if (contentPixmap()) {
        m_content_tex_partition->setPixmap(contentPixmap(), false, realWidth(), realHeight(), depth());
    }

    if (clipShapeChanged()) {
        updateShape();
    }

    clearDamage();
}

// Updates window's geometry.
void OpenGLWindow::updateGeometry() {
    BaseCompWindow::updateGeometry();
    updateWindowPos();
}

// Updates the window's shape.
void OpenGLWindow::updateShape() {
    BaseCompWindow::updateShape();

    Pixmap shape_pixmap = XCreatePixmap(display(), window(), realWidth(), realHeight(), depth());

    GC gc = XCreateGC(display(), shape_pixmap, 0, 0);
    XSetGraphicsExposures(display(), gc, False);
    XSetPlaneMask(display(), gc, 0xffffffff);

    XSetForeground(display(), gc, 0x00000000);
    XFillRectangle(display(), shape_pixmap, gc, 0, 0, realWidth(), realHeight());

    XSetForeground(display(), gc, 0xffffffff);
    XSetClipRectangles(display(), gc, 0, 0, clipShapeRects(), clipShapeRectCount(), Unsorted);
    XFillRectangle(display(), shape_pixmap, gc, 0, 0, realWidth(), realHeight());

    XFreeGC(display(), gc);

    m_shape_tex_partition->setPixmap(shape_pixmap, true, realWidth(), realHeight(), depth());
}

// Updates the window position vertex array.
void OpenGLWindow::updateWindowPos() {
    m_window_pos_buffers = partitionSpaceToBuffers(openGLScreen(), x(), y(), realWidth(), realHeight());
}
