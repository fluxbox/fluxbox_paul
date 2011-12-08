/** XRenderResources.cc file for the fluxbox compositor. */

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


#include "XRenderResources.hh"

#include "XRenderScreen.hh"

using namespace FbCompositor;


//--- XRENDER PICTURE WRAPPER --------------------------------------------------

//------- CONSTRUCTORS AND DESTRUCTORS -----------------------------------------

// Constructor.
XRenderPicture::XRenderPicture(const XRenderScreen &screen, XRenderPictFormat *pict_format, const char *pict_filter) :
    m_drawable(None),
    m_gc(None),
    m_picture(None),
    m_resources_managed(false),
    m_pict_filter(pict_filter),
    m_pict_format(pict_format),
    m_screen(screen) {

    m_display = (Display*)(screen.display());
}

// Destructor.
XRenderPicture::~XRenderPicture() {
    freeResources();
}


//------- MUTATORS -------------------------------------------------------------

// Associate the picture with the given pixmap.
void XRenderPicture::setPixmap(Pixmap pixmap, bool manage_pixmap, XRenderPictureAttributes pa, long pa_mask) {
    if (m_drawable != pixmap) {
        freeResources();

        m_drawable = pixmap;
        m_gc = XCreateGC(m_display, pixmap, 0, NULL);

        m_picture = XRenderCreatePicture(m_display, pixmap, m_pict_format, pa_mask, &pa);
        XRenderSetPictureFilter(m_display, m_picture, m_pict_filter, NULL, 0);
    }

    m_resources_managed = manage_pixmap;
}

// Associate the picture with the given window.
void XRenderPicture::setWindow(Window window, XRenderPictureAttributes pa, long pa_mask) {
    if (m_drawable != window) {
        freeResources();

        m_drawable = window;
        m_gc = XCreateGC(m_display, window, 0, NULL);

        m_picture = XRenderCreatePicture(m_display, window, m_pict_format, pa_mask, &pa);
        XRenderSetPictureFilter(m_display, m_picture, m_pict_filter, NULL, 0);
    }

    m_resources_managed = false;
}


// Reset the picture's transformation matrix.
void XRenderPicture::resetPictureTransform() {
    XTransform transform = { {
        { XDoubleToFixed(1.0), XDoubleToFixed(0.0), XDoubleToFixed(0.0) },
        { XDoubleToFixed(0.0), XDoubleToFixed(1.0), XDoubleToFixed(0.0) },
        { XDoubleToFixed(0.0), XDoubleToFixed(0.0), XDoubleToFixed(1.0) }
    } };
    setPictureTransform(transform);
}

// Scale the picture by the given inverse quotients.
void XRenderPicture::scalePicture(double x_factor_inv, double y_factor_inv) {
    XTransform transform = { {
        { XDoubleToFixed(x_factor_inv), XDoubleToFixed(0.0), XDoubleToFixed(0.0) },
        { XDoubleToFixed(0.0), XDoubleToFixed(y_factor_inv), XDoubleToFixed(0.0) },
        { XDoubleToFixed(0.0), XDoubleToFixed(0.0), XDoubleToFixed(1.0) }
    } };
    setPictureTransform(transform);
}

// Set the picture's transformation matrix.
void XRenderPicture::setPictureTransform(const XTransform &transform) {
    XRenderSetPictureTransform(m_display, m_picture, (XTransform*)(&transform));
}


//--- OTHER FUNCTIONS ----------------------------------------------------------

// Free held resources, if any.
void XRenderPicture::freeResources() {
    if (m_picture) {
        XRenderFreePicture(m_display, m_picture);
        m_picture = None;
    }
    if (m_gc) {
        XFreeGC(m_display, m_gc);
        m_gc = None;
    }

    if (m_resources_managed && m_drawable) {
        XFreePixmap(m_display, m_drawable);     // Windows will never be managed.
        m_drawable = None;
    }
}
