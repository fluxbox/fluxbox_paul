/** PreviewPlugin.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_PLUGIN_OPENGL_PREVIEW_PREVIEWPLUGIN_HH
#define FBCOMPOSITOR_PLUGIN_OPENGL_PREVIEW_PREVIEWPLUGIN_HH


#include "Enumerations.hh"
#include "OpenGLPlugin.hh"
#include "OpenGLResources.hh"
#include "OpenGLShaders.hh"
#include "TickTracker.hh"

#include "FbTk/FbString.hh"

#include <GL/glew.h>
#include <GL/gl.h>

#include <X11/Xlib.h>

#include <map>
#include <vector>


namespace FbCompositor {

    class BaseScreen;
    class OpenGLScreen;
    class OpenGLWindow;


    /**
     * Provides window preview feature for the iconbar.
     */
    class PreviewPlugin : public OpenGLPlugin {
        struct PreviewWindowData;

    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        PreviewPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args);

        /** Destructor. */
        ~PreviewPlugin();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the name of the plugin. */
        const char *pluginName() const;


        /** \returns the additional source code for the fragment shader. */
        const char *fragmentShader() const;

        /** \returns the additional source code for the vertex shader. */
        const char *vertexShader() const;


        //--- WINDOW EVENT CALLBACKS -------------------------------------------

        /** Called, whenever a new window is created. */
        void windowCreated(const BaseCompWindow &window);

        /** Called, whenever a window is destroyed. */
        void windowDestroyed(const BaseCompWindow &window);


        //--- RENDERING ACTIONS ------------------------------------------------

        /** Extra rendering actions and jobs. */
        const std::vector<OpenGLRenderingJob> &extraRenderingActions();


    private :
        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** Update the preview window. */
        void updatePreviewWindow(PreviewWindowData &win_preview);


        //--- GENERAL RENDERING VARIABLES --------------------------------------

        /** Vector, containing the plugin's extra rendering jobs. */
        std::vector<OpenGLRenderingJob> m_extra_jobs;


        /** Timer that signals when the preview window should appear. */
        TickTracker m_tick_tracker;


        //--- PREVIEW WINDOW DATA ----------------------------------------------

        /** Holds data about the preview window. */
        struct PreviewWindowData {
            const OpenGLWindow &window;     ///< The corresponding window object.
            OpenGLRenderingJob job;         ///< Rendering job of this preview object.
        };

        /** A list of potential preview windows. */
        std::map<Window, PreviewWindowData> m_preview_data;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the name of the plugin.
    inline const char *PreviewPlugin::pluginName() const {
        return "preview";
    }
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

/** Creates a plugin object. */
extern "C" FbCompositor::BasePlugin *createPlugin(const FbCompositor::BaseScreen &screen, const std::vector<FbTk::FbString> &args);

/** \returns the type of the plugin. */
extern "C" FbCompositor::PluginType pluginType();


#endif  // FBCOMPOSITOR_PLUGIN_OPENGL_PREVIEW_PREVIEWPLUGIN_HH
