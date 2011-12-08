/** XRenderWindow.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_XRENDERWINDOW_HH
#define FBCOMPOSITOR_XRENDERWINDOW_HH


#include "BaseCompWindow.hh"
#include "Exceptions.hh"
#include "XRenderResources.hh"

#include <X11/extensions/Xrender.h>


namespace FbCompositor {

    class InitException;
    class XRenderScreen;
    class XRenderWindow;


    /**
     * Manages windows in XRender rendering mode.
     */
    class XRenderWindow : public BaseCompWindow {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        XRenderWindow(const XRenderScreen &screen, Window window_xid, const char *pict_filter);

        /** Destructor. */
        ~XRenderWindow();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns an object, holding the window's contents as an XRender picture. */
        XRenderPicturePtr contentPicture() const;
        
        /** \returns an object, the window's mask picture. */
        XRenderPicturePtr maskPicture() const;


        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Update the window's contents. */
        void updateContents();

        /** Update window's property. */
        void updateProperty(Atom property, int state);


    protected:
        //--- PROTECTED WINDOW MANIPULATION ------------------------------------

        /** Update the window's clip shape. */
        void updateShape();


    private:
        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** Update the window's mask picture. */
        void updateMaskPicture();


        //--- RENDERING RELATED ------------------------------------------------

        /** The window's content picture. */
        XRenderPicturePtr m_content_picture;

        /** The window's mask picture. */
        XRenderPicturePtr m_mask_picture;


        /** The picture filter. */
        const char *m_pict_filter;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the window's contents as an XRender picture.
    inline XRenderPicturePtr XRenderWindow::contentPicture() const {
        return m_content_picture;
    }

    // Returns the window's mask picture.
    inline XRenderPicturePtr XRenderWindow::maskPicture() const {
        return m_mask_picture;
    }
}

#endif  // FBCOMPOSITOR_XRENDERWINDOW_HH
