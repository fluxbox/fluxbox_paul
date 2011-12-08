/** OpenGLPlugin.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_OPENGLPLUGIN_HH
#define FBCOMPOSITOR_OPENGLPLUGIN_HH


#include "BasePlugin.hh"
#include "OpenGLResources.hh"
#include "OpenGLShaders.hh"

#include "FbTk/FbString.hh"

#include <GL/glew.h>
#include <GL/gl.h>

#include <X11/Xlib.h>

#include <vector>


namespace FbCompositor {

    class BaseScreen;
    class OpenGLScreen;
    class OpenGLWindow;


    //--- RENDERING JOB INFORMATION --------------------------------------------

    /**
     * Information about an extra rendering job.
     */
    struct OpenGLRenderingJob {
        OpenGLBufferPtr prim_pos_buffer;            ///< Primitive's position buffer.
        OpenGLBufferPtr main_tex_coord_buffer;      ///< Main texture's position buffer.
        OpenGLBufferPtr shape_tex_coord_buffer;     ///< Shape texture's position buffer.
        OpenGL2DTexturePtr main_texture;            ///< Main texture.
        OpenGL2DTexturePtr shape_texture;           ///< Shape texture.
        GLfloat alpha;                              ///< Rendering's alpha.

        OpenGLShaderInitializer *shader_init;       ///< Shader initializer.
        OpenGLShaderDeinitializer *shader_deinit;   ///< Shader deinitializer.
    };


    //--- OPENGL PLUGIN BASE CLASS ---------------------------------------------

    /**
     * Plugin for OpenGL renderer.
     */
    class OpenGLPlugin : public BasePlugin {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args);

        /** Destructor. */
        virtual ~OpenGLPlugin();


        //--- OTHER INITIALIZATION ---------------------------------------------

        /** Initialize OpenGL-specific code. */
        virtual void initOpenGL(OpenGLShaderProgramPtr shader_program);


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the screen object, cast into the correct class. */
        const OpenGLScreen &openGLScreen() const;


        /** \returns the additional source code for the fragment shader. */
        virtual const char *fragmentShader() const = 0;

        /** \returns the additional source code for the vertex shader. */
        virtual const char *vertexShader() const = 0;


        //--- RENDERING ACTIONS ------------------------------------------------

        /** Background rendering initialization. */
        virtual void backgroundRenderInit(int part_id);

        /** Background rendering cleanup. */
        virtual void backgroundRenderCleanup(int part_id);

        /** Post background rendering actions. */
        virtual const std::vector<OpenGLRenderingJob> &postBackgroundRenderActions();


        /** Pre window rendering actions and jobs. */
        virtual const std::vector<OpenGLRenderingJob> &preWindowRenderActions(const OpenGLWindow &window);

        /** Window rendering initialization. */
        virtual void windowRenderInit(const OpenGLWindow &window, int part_id);

        /** Window rendering cleanup. */
        virtual void windowRenderCleanup(const OpenGLWindow &window, int part_id);

        /** Post window rendering actions and jobs. */
        virtual const std::vector<OpenGLRenderingJob> &postWindowRenderActions(const OpenGLWindow &window);


        /** Reconfigure rectangle rendering initialization. */
        virtual void recRectRenderInit(const XRectangle &rec_rect);

        /** Reconfigure rectangle rendering cleanup. */
        virtual void recRectRenderCleanup(const XRectangle &rec_rect);


        /** Extra rendering actions and jobs. */
        virtual const std::vector<OpenGLRenderingJob> &extraRenderingActions();

        /** Post extra rendering actions. */
        virtual void postExtraRenderingActions();


        /** Null rendering job initialization. */
        virtual void nullRenderInit();
    };
}

#endif  // FBCOMPOSITOR_OPENGLPLUGIN_HH
