/** XRenderPlugin.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_XRENDERPLUGIN_HH
#define FBCOMPOSITOR_XRENDERPLUGIN_HH

#include "BasePlugin.hh"
#include "Enumerations.hh"
#include "Exceptions.hh"
#include "XRenderResources.hh"

#include "FbTk/FbString.hh"

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

#include <vector>


namespace FbCompositor {

    class BasePlugin;
    class BaseScreen;
    class InitException;
    class XRenderScreen;
    class XRenderWindow;
    class RenderingException;


    //--- SUPPORTING STRUCTURES AND CLASSES ------------------------------------

    /**
     * A rendering job.
     */
    struct XRenderRenderingJob {
        int operation;                      ///< Compositing operation to use.
        XRenderPicturePtr source_picture;   ///< Picture to render.
        XRenderPicturePtr mask_picture;     ///< Mask picture to use.
        int source_x;                       ///< X offset on the source picture.
        int source_y;                       ///< Y offset on the source picture.
        int mask_x;                         ///< X offset on the mask picture.
        int mask_y;                         ///< Y offset on the mask picture.
        int destination_x;                  ///< X offset on the destination picture.
        int destination_y;                  ///< Y offset on the destination picture.
        int width;                          ///< Width of the picture to render.
        int height;                         ///< Height of the picture to render.
    };


    //--- XRENDER PLUGIN BASE CLASS --------------------------------------------

    /**
     * Plugin for XRender renderer.
     */
    class XRenderPlugin : public BasePlugin {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        XRenderPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args);

        /** Destructor. */
        virtual ~XRenderPlugin();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the screen object, cast into the correct class. */
        const XRenderScreen &xrenderScreen() const;


        //--- RENDERING ACTIONS ------------------------------------------------

        /** Rectangles that the plugin wishes to damage. */
        virtual const std::vector<XRectangle> &damagedAreas();


        /** Post background rendering actions and jobs. */
        virtual const std::vector<XRenderRenderingJob> &postBackgroundRenderingActions();


        /** Pre window rendering actions and jobs. */
        virtual const std::vector<XRenderRenderingJob> &preWindowRenderingActions(const XRenderWindow &window);

        /** Window rendering job initialization. */
        virtual void windowRenderingJobInit(const XRenderWindow &window, XRenderRenderingJob &job);

        /** Post window rendering actions and jobs. */
        virtual const std::vector<XRenderRenderingJob> &postWindowRenderingActions(const XRenderWindow &window);


        /** Reconfigure rectangle rendering job initialization. */
        virtual void recRectRenderingJobInit(XRectangle &rect_return, GC gc);


        /** Extra rendering actions and jobs. */
        virtual const std::vector<XRenderRenderingJob> &extraRenderingActions();

        /** Post extra rendering actions. */
        virtual void postExtraRenderingActions();
    };
}

#endif  // FBCOMPOSITOR_XRENDERPLUGIN_HH
