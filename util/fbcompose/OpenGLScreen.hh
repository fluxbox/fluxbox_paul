/** OpenGLScreen.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_XRENDERAUTOSCREEN_HH
#define FBCOMPOSITOR_XRENDERAUTOSCREEN_HH


#include "BaseScreen.hh"
#include "OpenGLPlugin.hh"
#include "OpenGLShaders.hh"
#include "OpenGLTexPartitioner.hh"
#include "OpenGLWindow.hh"

#include <GL/glxew.h>
#include <GL/glx.h>


namespace FbCompositor {

    class BaseCompWindow;
    class CompositorConfig;


    /**
     * Manages screen in OpenGL rendering mode.
     */
    class OpenGLScreen : public BaseScreen {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLScreen(int screen_number, const CompositorConfig &config);

        /** Destructor. */
        ~OpenGLScreen();


        //--- OTHER INITIALIZATION ---------------------------------------------

        /** Initializes the screen's plugins. */
        void initPlugins(const CompositorConfig &config);


        //--- DEFAULT OPENGL OBJECT ACCESSORS ----------------------------------

        /** \returns the default element buffer (rectangle, corners in order of NW, NE, SW, SE). */
        OpenGLBufferPtr defaultElementBuffer() const;

        /** \returns the default primitive position buffer (four corners of the root window). */
        OpenGLBufferPtr defaultPrimPosBuffer() const;

        /** \returns the default texture position buffer (the whole texture). */
        OpenGLBufferPtr defaultTexCoordBuffer() const;


        /** \returns the default black texture. */
        OpenGL2DTexturePtr blackTexture() const;

        /** \returns the default white texture. */
        OpenGL2DTexturePtr whiteTexture() const;


        //--- OTHER ACCESSORS --------------------------------------------------

        /** \return the main GLX context. */
        GLXContext context() const;

        /** \returns the main GLXFBConfig. */
        GLXFBConfig fbConfig() const;

        /** \returns maximum supported texture size. */
        int maxTextureSize() const;


        //--- SCREEN MANIPULATION ----------------------------------------------

        /** Notifies the screen of the background change. */
        void setRootPixmapChanged();

        /** Notifies the screen of a root window change. */
        void setRootWindowSizeChanged();


        //--- SCREEN RENDERING -------------------------------------------------

        /** Renders the screen's contents. */
        void renderScreen();


    protected:
        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Creates a window object from its XID. */
        BaseCompWindow *createWindowObject(Window window);


    private:
        //--- INITIALIZATION FUNCTIONS -----------------------------------------

        /** Creates OpenGL resources. */
        void createResources();

        /** Early initialization of GLX function pointers. */
        void earlyInitGLXPointers();

        /** Finds the maximum usable texture size. */
        void findMaxTextureSize();

        /** Finishes the initialization of the rendering context and surface. */
        void finishRenderingInit();

        /** Initializes GLEW. */
        void initGlew();

        /** Finish plugin initialization. */
        void initPlugins();

        /** Initializes the rendering context. */
        void initRenderingContext();

        /** Initializes the rendering surface. */
        void initRenderingSurface();


        //--- OTHER FUNCTIONS --------------------------------------------------

        /** Renews the background texture. */
        void updateBackgroundTexture();

        /** React to the geometry change of the root window. */
        void updateOnRootWindowResize();


        //--- RENDERING FUNCTIONS ----------------------------------------------

        /** Render the desktop background. */
        void renderBackground();

        /** Perform extra rendering jobs from plugins. */
        void renderExtraJobs();

        /** Render the reconfigure rectangle. */
        void renderReconfigureRect();

        /** Render a particular window onto the screen. */
        void renderWindow(OpenGLWindow &window);


        /** Execute multiple rendering jobs. */
        void executeMultipleJobs(OpenGLPlugin *plugin, const std::vector<OpenGLRenderingJob> &jobs);

        /** Execute a given rendering job. */
        void executeRenderingJob(const OpenGLRenderingJob &job);

        /** Render something onto the screen. */
        void render(GLenum rendering_mode, OpenGLBufferPtr prim_pos_buffer,
                    OpenGLBufferPtr main_tex_coord_buffer, OpenGL2DTexturePtr main_texture,
                    OpenGLBufferPtr shape_tex_coord_buffer, OpenGL2DTexturePtr shape_texture,
                    OpenGLBufferPtr element_buffer, GLuint element_count, GLfloat alpha);


        //--- MAIN RENDERING-RELATED VARIABLES ---------------------------------

        /** The main FBConfig. */
        GLXFBConfig m_fb_config;

        /** The GLX context. */
        GLXContext m_glx_context;

        /** Shaders. */
        OpenGLShaderProgramPtr m_shader_program;


        /** GLX handle to the rendering window. */
        GLXWindow m_glx_rendering_window;

        /** Rendering window. */
        Window m_rendering_window;

        /** Whether the root window has changed since the last update. */
        bool m_root_window_changed;


        //--- DESKTOP BACKGROUND RELATED ---------------------------------------

        /** The background texture. */
        OpenGL2DTexturePartitionPtr m_bg_texture;

        /** Position buffers of the background texture partitions. */
        std::vector<OpenGLBufferPtr> m_bg_pos_buffers;

        /** Whether the background changed since the last update. */
        bool m_bg_changed;


        //--- DEFAULT OPENGL ELEMENTS ------------------------------------------

        /** Default element buffer. */
        OpenGLBufferPtr m_default_element_buffer;

        /** Default primitive position buffer. */
        OpenGLBufferPtr m_default_prim_pos_buffer;

        /** Default texture position buffer. */
        OpenGLBufferPtr m_default_tex_coord_buffer;


        /** Default black texture. */
        OpenGL2DTexturePtr m_black_texture;

        /** Default white texture. */
        OpenGL2DTexturePtr m_white_texture;


        //--- RESIZE FRAME RELATED ---------------------------------------------

        /** The reconfigure rectangle element buffer. */
        OpenGLBufferPtr m_rec_rect_element_buffer;

        /** The reconfigure rectangle primitive position array buffer. */
        OpenGLBufferPtr m_rec_rect_line_pos_buffer;


        //--- OTHER VARIABLES --------------------------------------------------

        /** Whether we have a double-buffered window. */
        bool m_have_double_buffering;

        /** Maximum texture size. */
        int m_max_texture_size;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the default black texture.
    inline OpenGL2DTexturePtr OpenGLScreen::blackTexture() const {
        return m_black_texture;
    }

    // Return the main GLX context.
    inline GLXContext OpenGLScreen::context() const {
        return m_glx_context;
    }

    // Returns the default element buffer.
    inline OpenGLBufferPtr OpenGLScreen::defaultElementBuffer() const {
        return m_default_element_buffer;
    }

    // Returns the default primitive position buffer.
    inline OpenGLBufferPtr OpenGLScreen::defaultPrimPosBuffer() const {
        return m_default_prim_pos_buffer;
    }

    // Returns the default texture position buffer.
    inline OpenGLBufferPtr OpenGLScreen::defaultTexCoordBuffer() const {
        return m_default_tex_coord_buffer;
    }

    // Returns the main GLXFBConfig.
    inline GLXFBConfig OpenGLScreen::fbConfig() const {
        return m_fb_config;
    }

    // Returns maximum supported texture size.
    inline int OpenGLScreen::maxTextureSize() const {
        return m_max_texture_size;
    }

    // Returns the default white texture.
    inline OpenGL2DTexturePtr OpenGLScreen::whiteTexture() const {
        return m_white_texture;
    }
}

#endif  // FBCOMPOSITOR_XRENDERAUTOSCREEN_HH
