/** XRenderScreen.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_XRENDERSCREEN_HH
#define FBCOMPOSITOR_XRENDERSCREEN_HH


#include "BaseScreen.hh"

#include "XRenderPlugin.hh"
#include "XRenderResources.hh"

#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xlib.h>


namespace FbCompositor {

    class BaseScreen;
    class CompositorConfig;
    class XRenderScreen;
    class XRenderWindow;


    /**
     * Manages the screen in XRender rendering mode.
     */
    class XRenderScreen : public BaseScreen {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        XRenderScreen(int screen_number, const CompositorConfig &config);

        /** Destructor. */
        ~XRenderScreen();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the preferred filter for XRender Pictures. */
        const char *pictFilter() const;


        //--- SCREEN MANIPULATION ----------------------------------------------

        /** Notifies the screen of a background change. */
        void setRootPixmapChanged();

        /** Notifies the screen of a root window change. */
        void setRootWindowSizeChanged();


        //--- SCREEN RENDERING -------------------------------------------------

        /** Renders the screen's contents. */
        void renderScreen();


    protected:
        //--- SPECIALIZED WINDOW MANIPULATION FUNCTIONS ------------------------

        /** Creates a window object from its XID. */
        BaseCompWindow *createWindowObject(Window window);


    private:
        //--- INITIALIZATION FUNCTIONS -----------------------------------------

        /** Initializes the rendering surface. */
        void initRenderingSurface();


        //--- SCREEN MANIPULATION ----------------------------------------------

        /** Update the background picture. */
        void updateBackgroundPicture();


        //--- RENDERING FUNCTIONS ----------------------------------------------

        /** Clips the backbuffer picture to damaged area. */
        void clipBackBufferToDamage();

        /** Perform a rendering job on the back buffer picture. */
        void executeRenderingJob(const XRenderRenderingJob &job);

        /** Render the desktop wallpaper. */
        void renderBackground();

        /** Perform extra rendering jobs from plugins. */
        void renderExtraJobs();

        /** Render the reconfigure rectangle. */
        void renderReconfigureRect();

        /** Render a particular window onto the screen. */
        void renderWindow(XRenderWindow &window);

        /** Swap back and front buffers. */
        void swapBuffers();


        //--- MAIN RENDERING-RELATED VARIABLES ---------------------------------

        /** The rendering window. */
        Window m_rendering_window;

        /** The picture of the back buffer. */
        XRenderPicturePtr m_back_buffer_picture;

        /** The picture of the rendering window. */
        XRenderPicturePtr m_rendering_picture;


        /** A container for rectangle, damaged by plugins. */
        std::vector<XRectangle> m_plugin_damage_rects;

        /** Screen region, damaged by plugins. */
        XserverRegion m_plugin_damage;


        /** The picture of the root window. */
        XRenderPicturePtr m_root_picture;

        /** Whether the root window has changed since the last update. */
        bool m_root_changed;


        /** The picture filter to use. */
        const char *m_pict_filter;
    };


    //--- INLINE FUNCTIONS -------------------------------------------------

    // Returns the preferred filter for XRender Pictures.
    inline const char *XRenderScreen::pictFilter() const {
        return m_pict_filter;
    }
}

#endif  // FBCOMPOSITOR_XRENDERSCREEN_HH
