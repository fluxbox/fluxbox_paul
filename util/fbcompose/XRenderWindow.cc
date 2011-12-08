/** XRenderWindow.cc file for the fluxbox compositor. */

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


#include "XRenderWindow.hh"

#include "Atoms.hh"
#include "Logging.hh"
#include "XRenderScreen.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
XRenderWindow::XRenderWindow(const XRenderScreen &screen, Window window_xid, const char *pict_filter) :
    BaseCompWindow(static_cast<const BaseScreen&>(screen), window_xid, false),
    m_pict_filter(pict_filter) {

    XRenderPictFormat *content_pict_format = XRenderFindVisualFormat(display(), visual());
    m_content_picture = new XRenderPicture(screen, content_pict_format, m_pict_filter);

    XRenderPictFormat *mask_pict_format = XRenderFindStandardFormat(display(), PictStandardARGB32);
    m_mask_picture = new XRenderPicture(screen, mask_pict_format, m_pict_filter);
}

// Destructor.
XRenderWindow::~XRenderWindow() { }


//--- WINDOW MANIPULATION ------------------------------------------------------

// Update the window's contents.
void XRenderWindow::updateContents() {
    updateContentPixmap();
    if (contentPixmap()) {
        XRenderPictureAttributes pa;
        pa.subwindow_mode = IncludeInferiors;
        long pa_mask = CPSubwindowMode;

        m_content_picture->setPictFormat(XRenderFindVisualFormat(display(), visual()));
        m_content_picture->setPixmap(contentPixmap(), false, pa, pa_mask);
    }

    if (clipShapeChanged()) {
        updateShape();
    }

    clearDamage();
}

// Update window's property.
void XRenderWindow::updateProperty(Atom property, int state) {
    BaseCompWindow::updateProperty(property, state);

    if (property == Atoms::opacityAtom()) {
        updateMaskPicture();
    }
}


//--- PROTECTED WINDOW MANIPULATION --------------------------------------------

// Update the window's clip shape.
void XRenderWindow::updateShape() {
    BaseCompWindow::updateShape();
    updateMaskPicture();
}


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Update the window's mask picture.
void XRenderWindow::updateMaskPicture() {
    if ((m_mask_picture->pictureHandle() == None) || isResized()) {
        Pixmap mask_pixmap = XCreatePixmap(display(), window(), realWidth(), realHeight(), 32);
        m_mask_picture->setPixmap(mask_pixmap, true);
    }

    XRenderColor color = { 0, 0, 0, 0 };
    XRenderFillRectangle(display(), PictOpSrc, m_mask_picture->pictureHandle(), &color, 0, 0, realWidth(), realHeight());

    color.alpha = (unsigned long)((alpha() * 0xffff) / 255.0);
    XRenderFillRectangles(display(), PictOpSrc, m_mask_picture->pictureHandle(), &color, clipShapeRects(), clipShapeRectCount());
}
