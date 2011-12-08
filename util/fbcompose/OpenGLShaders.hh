/** OpenGLShaders.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_OPENGLSHADERS_HH
#define FBCOMPOSITOR_OPENGLSHADERS_HH


#include "FbTk/Command.hh"
#include "FbTk/RefCount.hh"

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>


namespace FbCompositor {

    class BasePlugin;


    //--- SHADER INITIALIZERS --------------------------------------------------

    /** Rendering job initialization functor.  */
    typedef FbTk::Command<void> OpenGLShaderInitializer;

    /**
     * Null initialization action.
     */
    class NullInitializer : public OpenGLShaderInitializer {
    public :
        virtual ~NullInitializer() { }
        void execute() { }
    };


    //--- SHADER DEINITIALIZERS ------------------------------------------------

    /** Rendering job cleanup functor. */
    typedef FbTk::Command<void> OpenGLShaderDeinitializer;

    /** 
     * Null cleanup action.
     */
    class NullDeinitializer : public OpenGLShaderDeinitializer {
    public :
        virtual ~NullDeinitializer() { }
        void execute() { }
    };


    //--- SHADER PROGRAM WRAPPER -----------------------------------------------

    /**
     * OpenGL shader program wrapper.
     */
    class OpenGLShaderProgram {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------
        
        /** Constructor. */
        OpenGLShaderProgram(const std::vector<BasePlugin*> &plugins);

        /** Destructor. */
        ~OpenGLShaderProgram();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns handle to the shader program. */
        GLuint programHandle() const;


        /** \returns location of the given attribute. */
        GLuint getAttributeLocation(const char *attrib_name);

        /** \returns location of the fb_InitMainTexCoord attribute. */
        GLuint mainTexCoordAttrib() const;

        /** \returns location of the fb_InitPrimPos attribute. */
        GLuint primPosAttrib() const;

        /** \returns location of the fb_InitShapeTexCoord attribute. */
        GLuint shapeTexCoordAttrib() const;


        /** \returns location of the given uniform. */
        GLuint getUniformLocation(const char *uniform_name);

        /** \returns location of the fb_Alpha uniform. */
        GLuint alphaUniform() const;
        
        /** \returns location of the fb_MainTexture uniform. */
        GLuint mainTexUniform() const;

        /** \returns location of the fb_ShapeTexture uniform. */
        GLuint shapeTexUniform() const;


        //--- SHADER MANIPULATION ----------------------------------------------

        /** Uses the current shader program. */
        void use();


    private :
        //--- INITIALIZATION FUNCTIONS -----------------------------------------

        /** Creates a shader. */
        GLuint createShader(GLenum shader_type, GLint source_length, const GLchar *source);

        /** Creates a shader program. */
        GLuint createShaderProgram(GLuint vertex_shader, GLuint fragment_shader);


        //--- MAIN VARIABLES ---------------------------------------------------

        /** The vertex shader. */
        GLuint m_vertex_shader;

        /** The fragment shader. */
        GLuint m_fragment_shader;
        
        /** The shader program. */
        GLuint m_shader_program;


        /** Location of the fb_InitMainTexCoord attribute. */
        GLuint m_main_tex_coord_attrib;

        /** Location of the fb_InitPrimPos attribute. */
        GLuint m_prim_pos_attrib;

        /** Location of the fb_InitShapeTexCoord attribute. */
        GLuint m_shape_tex_coord_attrib;


        /** Location of the fb_Alpha uniform. */
        GLuint m_alpha_uniform;
        
        /** Location of the fb_MainTexture uniform. */
        GLuint m_main_tex_uniform;

        /** Location of the fb_ShapeTexture uniform. */
        GLuint m_shape_tex_uniform;
    };


    //--- INLINE FUNCTIONS -------------------------------------------------

    // Returns location of the fb_Alpha uniform.
    inline GLuint OpenGLShaderProgram::alphaUniform() const {
        return m_alpha_uniform;
    }

    // Returns location of the given attribute.
    inline GLuint OpenGLShaderProgram::getAttributeLocation(const char *attrib_name) {
        return glGetAttribLocation(m_shader_program, attrib_name);
    }

    // Returns location of the given uniform.
    inline GLuint OpenGLShaderProgram::getUniformLocation(const char *uniform_name) {
        return glGetUniformLocation(m_shader_program, uniform_name);
    }

    // Returns location of the fb_InitMainTexCoord attribute.
    inline GLuint OpenGLShaderProgram::mainTexCoordAttrib() const {
        return m_main_tex_coord_attrib;
    }

    // Returns location of the fb_MainTexture uniform.
    inline GLuint OpenGLShaderProgram::mainTexUniform() const {
        return m_main_tex_uniform;
    }

    // Returns location of the fb_InitPrimPos attribute.
    inline GLuint OpenGLShaderProgram::primPosAttrib() const {
        return m_prim_pos_attrib;
    }

    // Returns the handle to the shader program.
    inline GLuint OpenGLShaderProgram::programHandle() const {
        return m_shader_program;
    }

    // Returns location of the fb_InitShapeTexCoord attribute.
    inline GLuint OpenGLShaderProgram::shapeTexCoordAttrib() const {
        return m_shape_tex_coord_attrib;
    }

    // Returns location of the fb_ShapeTexture uniform.
    inline GLuint OpenGLShaderProgram::shapeTexUniform() const {
        return m_shape_tex_uniform;
    }

    // Uses the current shader program.
    inline void OpenGLShaderProgram::use() {
        glUseProgram(m_shader_program);
    }


    //--- TYPEDEFS ---------------------------------------------------------

    typedef FbTk::RefCount<OpenGLShaderProgram> OpenGLShaderProgramPtr;
}

#endif  // FBCOMPOSITOR_OPENGLSHADERS_HH
