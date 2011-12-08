/** XRenderResources.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_XRENDERRESOURCES_HH
#define FBCOMPOSITOR_XRENDERRESOURCES_HH


#include "config.h"

#include "FbTk/RefCount.hh"

#include <X11/extensions/Xrender.h>
#include <X11/Xlib.h>


namespace FbCompositor {

    class XRenderScreen;


    //--- XRENDER PICTURE WRAPPER ----------------------------------------------

    /**
     * XRender picture wrapper.
     */
    class XRenderPicture {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        XRenderPicture(const XRenderScreen &screen, XRenderPictFormat *pict_format, const char *pict_filter);

        /** Destructor. */
        ~XRenderPicture();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the handle of the picture's drawable. */
        Drawable drawableHandle() const;

        /** \returns the GC of the picture's drawable. */
        GC gcHandle() const;

        /** \returns the handle of the picture held. */
        Picture pictureHandle() const;


        //--- MUTATORS ---------------------------------------------------------

        /** Set a new PictFormat. */
        void setPictFormat(XRenderPictFormat *pict_format);


        /** Associate the picture with the given pixmap. */
        void setPixmap(Pixmap pixmap, bool manage_pixmap, XRenderPictureAttributes pa = XRenderPictureAttributes(), long pa_mask = 0);

        /** Associate the picture with the given window. */
        void setWindow(Window window, XRenderPictureAttributes pa = XRenderPictureAttributes(), long pa_mask = 0);


        /** Reset the picture's transformation matrix. */
        void resetPictureTransform();

        /** Scale the picture by the given inverse quotients. */
        void scalePicture(double x_factor_inv, double y_factor_inv);

        /** Set the picture's transformation matrix. */
        void setPictureTransform(const XTransform &transform);

    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Constructor. */
        XRenderPicture(const XRenderPicture &other);

        /** Assignment operator. */
        XRenderPicture &operator=(const XRenderPicture &other);


        //--- OTHER FUNCTIONS --------------------------------------------------

        /** Free held resources, if any. */
        void freeResources();


        //--- INTERNALS --------------------------------------------------------

        /** The picture's drawable. */
        Drawable m_drawable;

        /** The picture's drawable's GC. */
        GC m_gc;

        /** The picture in question. */
        Picture m_picture;

        /** Whether the resources are managed by this object. */
        bool m_resources_managed;
        

        /** Picture filter to use. */
        const char *m_pict_filter;

        /** Picture format to use. */
        XRenderPictFormat *m_pict_format;


        /** Current connection to the X server. */
        Display *m_display;

        /** The screen that manages the current picture. */
        const XRenderScreen &m_screen;
    };


    // Returns the handle of the picture's drawable.
    inline Drawable XRenderPicture::drawableHandle() const {
        return m_drawable;
    }

    // Returns the GC of the picture's drawable.
    inline GC XRenderPicture::gcHandle() const {
        return m_gc;
    }

    // Returns the picture held.
    inline Picture XRenderPicture::pictureHandle() const {
        return m_picture;
    }

    // Set a new PictFormat.
    inline void XRenderPicture::setPictFormat(XRenderPictFormat *pict_format) {
        if (pict_format) {
            m_pict_format = pict_format;
        }
    }


    //--- TYPEDEFS -------------------------------------------------------------

    /** XRender picture wrapper smart pointer. */
    typedef FbTk::RefCount<XRenderPicture> XRenderPicturePtr;
}

#endif  // FBCOMPOSITOR_XRENDERRESOURCES_HH
