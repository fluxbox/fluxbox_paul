/** OpenGLResources.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_OPENGLRESOURCES_HH
#define FBCOMPOSITOR_OPENGLRESOURCES_HH


#include "FbTk/RefCount.hh"

#include <GL/glxew.h>
#include <GL/glx.h>

#include <X11/Xlib.h>


namespace FbCompositor {

    class OpenGLScreen;
    

    //--- OPENGL BUFFER WRAPPER ------------------------------------------------

    /**
     * A wrapper for OpenGL buffers.
     */
    class OpenGLBuffer {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGLBuffer(const OpenGLScreen &screen, GLenum target_buffer);

        /** Destructor. */
        ~OpenGLBuffer();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the handle to the buffer held. */
        GLuint handle() const;

        /** \returns the target of the buffer. */
        GLenum target() const;


        //--- MUTATORS ---------------------------------------------------------

        /** Bind the buffer to its target. */
        void bind();

        /** Loads the given data into the buffer. */
        void bufferData(int element_size, const GLvoid *data, GLenum usage_hint);

        /** Sets the buffer's contents to be the rectangle's coordinates on the screen. */
        void bufferPosRectangle(int screen_width, int screen_height, XRectangle rect);


    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Copy constructor. */
        OpenGLBuffer(const OpenGLBuffer &other);

        /** Assignment operator. */
        OpenGLBuffer &operator=(const OpenGLBuffer &other);


        //--- INTERNALS --------------------------------------------------------

        /** The buffer in question. */
        GLuint m_buffer;

        /** The target buffer. */
        GLenum m_target;


        /** Screen that manages this buffer. */
        const OpenGLScreen &m_screen;
    };


    // Bind the buffer to its target.
    inline void OpenGLBuffer::bind() {
        glBindBuffer(m_target, m_buffer);
    }

    // Loads the given data into the buffer.
    inline void OpenGLBuffer::bufferData(int element_size, const GLvoid *data, GLenum usage_hint) {
        bind();
        glBufferData(m_target, element_size, data, usage_hint);
    }

    // Returns the handle to the buffer held.
    inline GLuint OpenGLBuffer::handle() const {
        return m_buffer;
    }

    // Returns the target of the buffer.
    inline GLenum OpenGLBuffer::target() const {
        return m_target;
    }


    /** OpenGL buffer wrapper smart pointer. */
    typedef FbTk::RefCount<OpenGLBuffer> OpenGLBufferPtr;


    //--- OPENGL TEXTURE WRAPPER -----------------------------------------------

    /**
     * OpenGL texture wrapper.
     */
    class OpenGL2DTexture {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        OpenGL2DTexture(const OpenGLScreen &screen, bool swizzle_alpha_to_one);

        /** Destructor. */
        ~OpenGL2DTexture();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the handle to the texture held. */
        GLuint handle() const;


        /** \returns the height of the texture. */
        int height() const;

        /** \returns the width of the texture. */
        int width() const;


        //--- MUTATORS ---------------------------------------------------------

        /** Bind the texture to GL_TEXTURE_2D. */
        void bind();

        /** Sets the texture's contents to the given pixmap. */
        void setPixmap(Pixmap pixmap, bool manage_pixmap, int width, int height, bool force_direct = false);


    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Constructor. */
        OpenGL2DTexture(const OpenGL2DTexture &other);

        /** Assignment operator. */
        OpenGL2DTexture &operator=(const OpenGL2DTexture &other);


        //--- INTERNALS --------------------------------------------------------

        /** Whether this object manages the pixmap. */
        bool m_pixmap_managed;

        /** Pixmap of the texture's contents. */
        Pixmap m_pixmap;

        /** GLX pixmap of the texture's contents. */
        GLXPixmap m_glx_pixmap;

        /** The texture in question. */
        GLuint m_texture;


        /** Height of the texture. */
        int m_height;

        /** Width of the texture. */
        int m_width;


        /** Current connection to the X server. */
        Display *m_display;

        /** Screen that manages this texture. */
        const OpenGLScreen &m_screen;
    };


    // Bind the texture to its target.
    inline void OpenGL2DTexture::bind() {
        glBindTexture(GL_TEXTURE_2D, m_texture);
    }

    // Returns the handle to the texture held.
    inline GLuint OpenGL2DTexture::handle() const {
        return m_texture;
    }

    // Returns the height of the texture.
    inline int OpenGL2DTexture::height() const {
        return m_height;
    }

    // Returns the width of the texture.
    inline int OpenGL2DTexture::width() const {
        return m_width;
    }


    /** OpenGL texture wrapper smart pointer. */
    typedef FbTk::RefCount<OpenGL2DTexture> OpenGL2DTexturePtr;
}

#endif  // FBCOMPOSITOR_OPENGLRESOURCES_HH
