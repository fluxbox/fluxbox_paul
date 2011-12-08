/** FadePlugin.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_PLUGIN_OPENGL_FADE_FADEPLUGIN_HH
#define FBCOMPOSITOR_PLUGIN_OPENGL_FADE_FADEPLUGIN_HH


#include "Enumerations.hh"
#include "OpenGLPlugin.hh"
#include "OpenGLResources.hh"
#include "OpenGLTexPartitioner.hh"
#include "TickTracker.hh"

#include "FbTk/FbString.hh"

#include <GL/glew.h>
#include <GL/gl.h>

#include <X11/Xlib.h>

#include <vector>
#include <map>


namespace FbCompositor {

    class BaseScreen;
    class OpenGLWindow;


    //--- SHADER INITIALIZER ---------------------------------------------------

    /**
     * Fade shader initializer.
     */
    class FadeShaderInitializer : public OpenGLShaderInitializer {
    public :
        /** Default constructor. */
        FadeShaderInitializer() : m_alpha_uniform(0), m_alpha(0.0) { }

        /** Constructor. */
        FadeShaderInitializer(GLuint alphaUniform, GLfloat alpha) :
            m_alpha_uniform(alphaUniform), m_alpha(alpha) { }

        /** Destructor. */
        ~FadeShaderInitializer() { }

        /** Initialization code. */
        void execute() {
            glUniform1f(m_alpha_uniform, m_alpha);
        }

        /** Alpha mutator. */
        void setAlpha(GLfloat alpha) {
            m_alpha = alpha;
        }

        /** Uniform mutator. */
        void setUniform(GLuint alphaUniform) {
            m_alpha_uniform = alphaUniform;
        }

    private :
        /** Uniform location of the alpha value. */
        GLuint m_alpha_uniform;

        /** Alpha value to set. */
        GLfloat m_alpha;
    };


    //--- MAIN PLUGIN CLASS ----------------------------------------------------

    /**
     * A simple plugin that provides window fades.
     */
    class FadePlugin : public OpenGLPlugin {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        FadePlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args);

        /** Destructor. */
        ~FadePlugin();


        //--- OTHER INITIALIZATION ---------------------------------------------

        /** Initialize OpenGL-specific code. */
        void initOpenGL(OpenGLShaderProgramPtr shader_program);


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the name of the plugin. */
        const char *pluginName() const;

        
        /** \returns the additional source code for the fragment shader. */
        const char *fragmentShader() const;

        /** \returns the additional source code for the vertex shader. */
        const char *vertexShader() const;


        //--- WINDOW EVENT CALLBACKS -------------------------------------------

        /** Called, whenever a window becomes ignored. */
        void windowBecameIgnored(const BaseCompWindow &window);

        /** Called, whenever a window is mapped. */
        void windowMapped(const BaseCompWindow &window);

        /** Called, whenever a window is unmapped. */
        void windowUnmapped(const BaseCompWindow &window);


        //--- RENDERING ACTIONS ------------------------------------------------

        /** Background rendering initialization. */
        void backgroundRenderInit(int part_id);

        /** Window rendering initialization. */
        void windowRenderInit(const OpenGLWindow &window, int part_id);

        /** Reconfigure rectangle rendering initialization. */
        void recRectRenderInit(const XRectangle &rec_rect);


        /** Extra rendering actions and jobs. */
        const std::vector<OpenGLRenderingJob> &extraRenderingActions();

        /** Post extra rendering actions. */
        void postExtraRenderingActions();


        /** Null rendering job initialization. */
        void nullRenderInit();


    private :
        //--- GENERAL RENDERING VARIABLES --------------------------------------

        /** Location of the fade_Alpha uniform. */
        GLuint m_alpha_uniform_pos;


        /** Main fade initialization object. */
        FadeShaderInitializer m_shader_initializer;

        /** A vector, containing the extra rendering jobs. */
        std::vector<OpenGLRenderingJob> m_extra_jobs;


        //--- FADE SPECIFIC ----------------------------------------------------

        /** Holds the data about positive fades. */
        struct PosFadeData {
            int fade_alpha;         ///< Window's relative fade alpha.
            TickTracker timer;      ///< Timer that tracks the current fade.
        };

        /** A list of appearing (positive) fades. */
        std::map<Window, PosFadeData> m_positive_fades;


        /** Holds the data about negative fades. */
        struct NegFadeData {
            OpenGLRenderingJob job;     ///< The associated rendering job.
            int fade_alpha;             ///< Window's fade relative alpha.
            TickTracker timer;          ///< Timer that tracks the current fade.
            Window window_id;           ///< ID of the window being faded.
        };

        /** A list of disappearing (negative) fades. */
        std::vector<NegFadeData> m_negative_fades;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the name of the plugin.
    inline const char *FadePlugin::pluginName() const {
        return "fade";
    }
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

/** Creates a plugin object. */
extern "C" FbCompositor::BasePlugin *createPlugin(const FbCompositor::BaseScreen &screen, const std::vector<FbTk::FbString> &args);

/** \returns the type of the plugin. */
extern "C" FbCompositor::PluginType pluginType();


#endif  // FBCOMPOSITOR_PLUGIN_OPENGL_FADE_FADEPLUGIN_HH
