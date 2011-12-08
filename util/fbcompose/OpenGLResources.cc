/** OpenGLResources.cc file for the fluxbox compositor. */

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


#include "OpenGLResources.hh"

#include "Logging.hh"
#include "OpenGLScreen.hh"
#include "OpenGLUtility.hh"
#include "Utility.hh"

using namespace FbCompositor;


//--- CONSTANTS ----------------------------------------------------------------

// Attributes of the textures' GLX pixmaps.
const int TEX_PIXMAP_ATTRIBUTES[] = {
#ifdef GLXEW_EXT_texture_from_pixmap
    GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
    GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT,
    None
#else
    None
#endif  // GLXEW_EXT_texture_from_pixmap
};


//--- OPENGL BUFFER WRAPPER ----------------------------------------------------

//------- CONSTRUCTORS AND DESTRUCTORS -----------------------------------------

// Constructor.
OpenGLBuffer::OpenGLBuffer(const OpenGLScreen &screen, GLenum target_buffer) :
    m_screen(screen) {

    m_target = target_buffer;

    glGenBuffers(1, &m_buffer);
}

// Destructor.
OpenGLBuffer::~OpenGLBuffer() {
    glDeleteBuffers(1, &m_buffer);
}


//------- MUTATORS -------------------------------------------------------------

// Sets the buffer's contents to be the rectangle's coordinates on the screen.
void OpenGLBuffer::bufferPosRectangle(int screen_width, int screen_height, XRectangle rect) {
    static GLfloat x_low, x_high, y_low, y_high;
    static GLfloat temp_pos_array[8];

    toOpenGLCoords(screen_width, screen_height, rect, &x_low, &x_high, &y_low, &y_high);

    temp_pos_array[0] = temp_pos_array[4] = x_low;
    temp_pos_array[2] = temp_pos_array[6] = x_high;
    temp_pos_array[1] = temp_pos_array[3] = y_low;
    temp_pos_array[5] = temp_pos_array[7] = y_high;
    
    bufferData(sizeof(temp_pos_array), (const GLvoid*)(temp_pos_array), GL_STATIC_DRAW);
}


//--- OPENGL TEXTURE WRAPPER ---------------------------------------------------

//------- CONSTRUCTORS AND DESTRUCTORS -----------------------------------------

// Constructor.
OpenGL2DTexture::OpenGL2DTexture(const OpenGLScreen &screen, bool swizzle_alpha_to_one) :
    m_screen(screen) {

    m_display = (Display*)(screen.display());
    m_glx_pixmap = 0;
    m_pixmap = None;
    m_pixmap_managed = false;

    glGenTextures(1, &m_texture);
    bind();

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (swizzle_alpha_to_one) {
#ifdef GL_ARB_texture_swizzle
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE);
#else
#ifdef GL_EXT_texture_swizzle
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A_EXT, GL_ONE);
#endif  // GL_EXT_texture_swizzle
#endif  // GL_ARB_texture_swizzle
    }
}

// Destructor.
OpenGL2DTexture::~OpenGL2DTexture() {
    glDeleteTextures(1, &m_texture);

    if (m_glx_pixmap) {
        glXDestroyPixmap(m_display, m_glx_pixmap);
    }
    if (m_pixmap_managed && m_pixmap) {
        XFreePixmap(m_display, m_pixmap);
    }
}


//------- MUTATORS -------------------------------------------------------------

// Sets the texture's contents to the given pixmap.
void OpenGL2DTexture::setPixmap(Pixmap pixmap, bool manage_pixmap, int width, int height, bool force_direct) {
    bind();

    if (m_pixmap != pixmap) {
#ifdef GLXEW_EXT_texture_from_pixmap
        if (m_glx_pixmap) {
            glXReleaseTexImageEXT(m_display, m_glx_pixmap, GLX_BACK_LEFT_EXT);
            glXDestroyPixmap(m_display, m_glx_pixmap);
            m_glx_pixmap = 0;
        }
#endif  // GLXEW_EXT_texture_from_pixmap

        if (m_pixmap_managed && m_pixmap) {
            XFreePixmap(m_display, m_pixmap);
            m_pixmap = None;
        }
    }

    m_height = height;
    m_pixmap_managed = manage_pixmap;
    m_pixmap = pixmap;
    m_width = width;

#ifdef GLXEW_EXT_texture_from_pixmap
    if (!force_direct) {
        if (!m_glx_pixmap) {
            m_glx_pixmap = glXCreatePixmap(m_display, m_screen.fbConfig(), m_pixmap, TEX_PIXMAP_ATTRIBUTES);
            glXBindTexImageEXT(m_display, m_glx_pixmap, GLX_BACK_LEFT_EXT, NULL);
        }
    } else 
#else
    MARK_PARAMETER_UNUSED(force_direct);
#endif  // GLXEW_EXT_texture_from_pixmap

    {
        XImage *image = XGetImage(m_display, pixmap, 0, 0, width, height, AllPlanes, ZPixmap);
        if (!image) {
            fbLog_info << "Could not create XImage for pixmap to texture conversion." << std::endl;
            return;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)(&(image->data[0])));
        XDestroyImage(image);
    }
}
