/** OpenGLScreen.cc file for the fluxbox compositor. */

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


#include "OpenGLScreen.hh"

#include "CompositorConfig.hh"
#include "Logging.hh"
#include "Exceptions.hh"
#include "OpenGLResources.hh"
#include "OpenGLUtility.hh"
#include "Utility.hh"

#include "FbTk/FbString.hh"

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>

#include <list>
#include <sstream>

using namespace FbCompositor;


//--- MACROS -------------------------------------------------------------------

// Macro for plugin iteration.
#define forEachPlugin(i, plugin)                                                       \
    (plugin) = ((pluginManager().plugins().size() > 0)                                 \
                   ? (dynamic_cast<OpenGLPlugin*>(pluginManager().plugins()[0]))       \
                   : NULL);                                                            \
    for(size_t (i) = 0;                                                                \
        ((i) < pluginManager().plugins().size());                                      \
        (i)++,                                                                         \
        (plugin) = (((i) < pluginManager().plugins().size())                           \
                       ? (dynamic_cast<OpenGLPlugin*>(pluginManager().plugins()[(i)])) \
                       : NULL))


//--- CONSTANTS ----------------------------------------------------------------

/** The preferred framebuffer configuration. */
const int PREFERRED_FBCONFIG_ATTRIBUTES[] = {
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT | GLX_PIXMAP_BIT,
    GLX_DOUBLEBUFFER, GL_TRUE,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
#ifdef GLXEW_EXT_texture_from_pixmap
    GLX_BIND_TO_TEXTURE_RGBA_EXT, GL_TRUE,
#endif  // GLXEW_EXT_texture_from_pixmap
    None
};

/** The fallback framebuffer configuration. */
const int FALLBACK_FBCONFIG_ATTRIBUTES[] = {
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT | GLX_PIXMAP_BIT,
    GLX_DOUBLEBUFFER, GL_FALSE,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
#ifdef GLXEW_EXT_texture_from_pixmap
    GLX_BIND_TO_TEXTURE_RGBA_EXT, GL_TRUE,
#endif  // GLXEW_EXT_texture_from_pixmap
    None
};


/** Default element array for texture rendering. */
const GLushort DEFAULT_ELEMENT_ARRAY[] = {
    0, 1, 2, 3
};

/** Default primitive position array for texture rendering. */
const GLfloat DEFAULT_PRIM_POS_ARRAY[] = {
    -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0
};

/** Default texture position array for texture rendering. */
const GLfloat DEFAULT_TEX_POS_ARRAY[] = {
    0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0
};


/** Element array for drawing the reconfigure rectangle. */
const GLushort RECONFIGURE_RECT_ELEMENT_ARRAY[] = {
    0, 1, 3, 2, 0
};


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
OpenGLScreen::OpenGLScreen(int screen_number, const CompositorConfig &config) :
    BaseScreen(screen_number, Plugin_OpenGL, config) {

    m_bg_changed = true;
    m_root_window_changed = false;

    earlyInitGLXPointers();
    initRenderingContext();
    initRenderingSurface();
    initGlew();
    finishRenderingInit();

    findMaxTextureSize();
    createResources();
    initPlugins();
}

// Destructor.
OpenGLScreen::~OpenGLScreen() {
    XUnmapWindow(display(), m_rendering_window);
    glXDestroyWindow(display(), m_glx_rendering_window);
    glXDestroyContext(display(), m_glx_context);
    XDestroyWindow(display(), m_rendering_window);
}


//--- INITIALIZATION FUNCTIONS -------------------------------------------------

// Early initialization of GLX functions.
// We need GLX functions to create an OpenGL context and initialize GLEW. But
// using GLXEW zeroes the pointers, since glewInit() initializes them. And we
// have to use GLXEW to have easy access to GLX's extensions. So, this function
// performs minimal function initialization - just enough to create a context.
void OpenGLScreen::earlyInitGLXPointers() {
    glXCreateNewContext = (PFNGLXCREATENEWCONTEXTPROC)glXGetProcAddress((GLubyte*)"glXCreateNewContext");
    if (!glXCreateNewContext) {
        throw InitException("Cannot initialize glXCreateNewContext function.");
    }

    glXChooseFBConfig = (PFNGLXCHOOSEFBCONFIGPROC)glXGetProcAddress((GLubyte*)"glXChooseFBConfig");
    if (!glXChooseFBConfig) {
        throw InitException("Cannot initialize glXChooseFBConfig function.");
    }

    glXGetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)glXGetProcAddress((GLubyte*)"glXGetVisualFromFBConfig");
    if (!glXGetVisualFromFBConfig) {
        throw InitException("Cannot initialize glXGetVisualFromFBConfig function.");
    }

    glXCreateWindow = (PFNGLXCREATEWINDOWPROC)glXGetProcAddress((GLubyte*)"glXCreateWindow");
    if (!glXCreateWindow) {
        throw InitException("Cannot initialize glXCreateWindow function.");
    }
}

// Initializes the rendering context.
void OpenGLScreen::initRenderingContext() {
    int fb_config_count;

    GLXFBConfig *fb_configs = glXChooseFBConfig(display(), screenNumber(), PREFERRED_FBCONFIG_ATTRIBUTES, &fb_config_count);
    m_have_double_buffering = true;

    if (!fb_configs) {
        fb_configs = glXChooseFBConfig(display(), screenNumber(), FALLBACK_FBCONFIG_ATTRIBUTES, &fb_config_count);
        m_have_double_buffering = false;

        fbLog_warn << "Could not get a double-buffered framebuffer config, trying single buffer. Expect tearing." << std::endl;

        if (!fb_configs) {
            throw InitException("Screen does not support the required framebuffer configuration.");
        }
    }

    m_fb_config = fb_configs[0];
    XFree(fb_configs);

    // Creating the GLX rendering context.
    m_glx_context = glXCreateNewContext(display(), m_fb_config, GLX_RGBA_TYPE, NULL, True);
    if (!m_glx_context) {
        throw InitException("Cannot create the OpenGL rendering context.");
    }
}

// Initializes the rendering surface.
void OpenGLScreen::initRenderingSurface() {
    // Creating an X window for rendering.
    Window comp_overlay = XCompositeGetOverlayWindow(display(), rootWindow().window());

    XVisualInfo *visual_info = glXGetVisualFromFBConfig(display(), m_fb_config);
    Colormap colormap = XCreateColormap(display(), rootWindow().window(), visual_info->visual, AllocNone);

    XSetWindowAttributes wa;
    wa.colormap = colormap;
    long wa_mask = CWColormap;

    m_rendering_window = XCreateWindow(display(), comp_overlay, 0, 0, rootWindow().width(), rootWindow().height(), 0,
                                       visual_info->depth, InputOutput, visual_info->visual, wa_mask, &wa);
    XmbSetWMProperties(display(), m_rendering_window, "fbcompose", "fbcompose", NULL, 0, NULL, NULL, NULL);
    XMapWindow(display(), m_rendering_window);

    // Make sure the overlays do not consume any input events.
    XserverRegion empty_region = XFixesCreateRegion(display(), NULL, 0);
    XFixesSetWindowShapeRegion(display(), comp_overlay, ShapeInput, 0, 0, empty_region);
    XFixesSetWindowShapeRegion(display(), m_rendering_window, ShapeInput, 0, 0, empty_region);
    XFixesDestroyRegion(display(), empty_region);

    ignoreWindow(comp_overlay);
    ignoreWindow(m_rendering_window);

    // Creating a GLX handle for the above window.
    m_glx_rendering_window = glXCreateWindow(display(), m_fb_config, m_rendering_window, NULL);
    if (!m_glx_rendering_window) {
        throw InitException("Cannot create the rendering surface.");
    }

    XFree(visual_info);
}

// Initializes GLEW.
void OpenGLScreen::initGlew() {
    glXMakeCurrent(display(), m_glx_rendering_window, m_glx_context);

    GLenum glew_err = glewInit();
    if(glew_err != GLEW_OK) {
        std::stringstream ss;
        ss << "GLEW Error: " << (const char*)(glewGetErrorString(glew_err));
        throw InitException(ss.str());
    }

    if (!GLEW_VERSION_2_1) {
        throw InitException("OpenGL 2.1 not available.");
    }
}

// Finishes the initialization of the rendering context and surface.
void OpenGLScreen::finishRenderingInit() {
    glXMakeCurrent(display(), m_glx_rendering_window, m_glx_context);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifndef GLXEW_EXT_texture_from_pixmap
    fbLog_warn << "GLX_EXT_texture_from_pixmap extension not found, expect a performance hit." << std::endl;
#endif

#ifndef GL_ARB_texture_swizzle
#ifndef GL_EXT_texture_swizzle
    fbLog_warn << "Could not find GL_ARB_texture_swizzle or GL_EXT_texture_swizzle extensions. Expect glitches." << std::endl;
#endif
#endif
}

// Finds the maximum usable texture size.
void OpenGLScreen::findMaxTextureSize() {
    GLint tex_size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tex_size);
    tex_size = (GLint)(largestSmallerPowerOf2((int)(tex_size)));

    while (tex_size > 0) {
        GLint width;

        glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, tex_size, tex_size, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
        glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);

        if (width == 0) {
            tex_size >>= 1;
        } else {
            break;
        }
    }

    m_max_texture_size = (int)(tex_size);
    if ((m_max_texture_size < (int)(rootWindow().width()))
            || (m_max_texture_size < (int)(rootWindow().height()))) {
        fbLog_warn << "Maximum supported OpenGL texture size on this machine is less than one "
                   << "of the root window's dimensions. There may be a performance hit." << std::endl;
    }
}

// Creates OpenGL resources.
void OpenGLScreen::createResources() {
    Pixmap pixmap;

    // Default element buffer.
    m_default_element_buffer = new OpenGLBuffer(*this, GL_ELEMENT_ARRAY_BUFFER);
    m_default_element_buffer->bufferData(sizeof(DEFAULT_ELEMENT_ARRAY), (const GLvoid*)(DEFAULT_ELEMENT_ARRAY), GL_STATIC_DRAW);

    // Default primitive position buffer.
    m_default_prim_pos_buffer = new OpenGLBuffer(*this, GL_ARRAY_BUFFER);
    m_default_prim_pos_buffer->bufferData(sizeof(DEFAULT_PRIM_POS_ARRAY), (const GLvoid*)(DEFAULT_PRIM_POS_ARRAY), GL_STATIC_DRAW);

    // Default texture position buffer.
    m_default_tex_coord_buffer = new OpenGLBuffer(*this, GL_ARRAY_BUFFER);
    m_default_tex_coord_buffer->bufferData(sizeof(DEFAULT_TEX_POS_ARRAY), (const GLvoid*)(DEFAULT_TEX_POS_ARRAY), GL_STATIC_DRAW);


    // Reconfigure rectangle position buffer.
    m_rec_rect_line_pos_buffer = new OpenGLBuffer(*this, GL_ARRAY_BUFFER);

    // Reconfigure rectangle element buffer.
    m_rec_rect_element_buffer = new OpenGLBuffer(*this, GL_ELEMENT_ARRAY_BUFFER);
    m_rec_rect_element_buffer->bufferData(sizeof(RECONFIGURE_RECT_ELEMENT_ARRAY),
                                          (const GLvoid*)(RECONFIGURE_RECT_ELEMENT_ARRAY), GL_STATIC_DRAW);


    // Background texture.
    m_bg_texture = new OpenGL2DTexturePartition(*this, true);

    // Background texture partition buffers.
    m_bg_pos_buffers = partitionSpaceToBuffers(*this, 0, 0, rootWindow().width(), rootWindow().height());


    // Plain black texture.
    pixmap = createSolidPixmap(*this, 1, 1, 0x00000000);
    m_black_texture = new OpenGL2DTexture(*this, false);
    m_black_texture->setPixmap(pixmap, false, 1, 1, true);
    XFreePixmap(display(), pixmap);

    // Plain white texture.
    pixmap = createSolidPixmap(*this, 1, 1, 0xffffffff);
    m_white_texture = new OpenGL2DTexture(*this, false);
    m_white_texture->setPixmap(pixmap, false, 1, 1, true);
    XFreePixmap(display(), pixmap);
}

// Finish plugin initialization.
void OpenGLScreen::initPlugins() {
    OpenGLPlugin *plugin = NULL;
    forEachPlugin(i, plugin) {
        plugin->initOpenGL(m_shader_program);
    }
}


//--- OTHER INITIALIZATION -----------------------------------------------------

// Initializes the screen's plugins.
void OpenGLScreen::initPlugins(const CompositorConfig &config) {
    BaseScreen::initPlugins(config);
    m_shader_program = new OpenGLShaderProgram(pluginManager().plugins());
}


//--- SCREEN MANIPULATION ------------------------------------------------------

// Notifies the screen of the background change.
void OpenGLScreen::setRootPixmapChanged() {
    BaseScreen::setRootPixmapChanged();
    m_bg_changed = true;
}

// Notifies the screen of a root window change.
void OpenGLScreen::setRootWindowSizeChanged() {
    BaseScreen::setRootWindowSizeChanged();
    m_root_window_changed = true;
}


//--- OTHER FUNCTIONS --------------------------------------------------

// Renews the background texture.
void OpenGLScreen::updateBackgroundTexture() {
    int depth = rootWindow().depth();
    if (!wmSetRootWindowPixmap()) {
        depth = 32;
    }

    m_bg_texture->setPixmap(rootWindowPixmap(), false, rootWindow().width(), rootWindow().height(), depth);
    m_bg_changed = false;
}

// React to the geometry change of the root window.
void OpenGLScreen::updateOnRootWindowResize() {
    XResizeWindow(display(), m_rendering_window, rootWindow().width(), rootWindow().height());

    std::list<BaseCompWindow*>::const_iterator it = allWindows().begin();
    while (it != allWindows().end()) {
        (dynamic_cast<OpenGLWindow*>(*it))->updateWindowPos();
        ++it;
    }

    m_bg_pos_buffers = partitionSpaceToBuffers(*this, 0, 0, rootWindow().width(), rootWindow().height());
    m_root_window_changed = false;
}


//--- WINDOW MANIPULATION ------------------------------------------------------

// Creates a window object from its XID.
BaseCompWindow *OpenGLScreen::createWindowObject(Window window) {
    OpenGLWindow *new_window = new OpenGLWindow(*this, window);
    return new_window;
}


//--- SCREEN RENDERING ---------------------------------------------------------

// Renders the screen's contents.
void OpenGLScreen::renderScreen() {
    // React to root window changes.
    if (m_root_window_changed) {
        updateOnRootWindowResize();
    }

    // Set up the rendering context.
    glXMakeCurrent(display(), m_glx_rendering_window, m_glx_context);
    m_shader_program->use();

    // Render background.
    renderBackground();

    // Render windows.
    std::list<BaseCompWindow*>::const_iterator it = allWindows().begin();
    while (it != allWindows().end()) {
        if (!(*it)->isIgnored() && (*it)->isMapped()) {
            renderWindow(*(dynamic_cast<OpenGLWindow*>(*it)));
        }
        ++it;
    }

    // Render reconfigure rectangle.
    if ((reconfigureRectangle().width != 0) && (reconfigureRectangle().height != 0)) {
        renderReconfigureRect();
    }

    // Execute any extra jobs from plugins.
    renderExtraJobs();

    // Finish.
    glFlush();
    if (m_have_double_buffering) {
        glXSwapBuffers(display(), m_glx_rendering_window);
    }
}


// A function to render the desktop background.
void OpenGLScreen::renderBackground() {
    OpenGLPlugin *plugin = NULL;

    // Update background texture if needed.
    if (m_bg_changed) {
        updateBackgroundTexture();
    }

    // Render background.
    for (size_t i = 0; i < m_bg_texture->partitions().size(); i++) {
        forEachPlugin(j, plugin) {
            plugin->backgroundRenderInit(i);
        }
        render(GL_TRIANGLE_STRIP, m_bg_pos_buffers[i],
               m_default_tex_coord_buffer, m_bg_texture->partitions()[i].texture,
               m_default_tex_coord_buffer, m_white_texture,
               m_default_element_buffer, 4, 1.0);
        forEachPlugin(j, plugin) {
            plugin->backgroundRenderCleanup(i);
        }
    }

    // Execute extra post background rendering jobs.
    forEachPlugin(i, plugin) {
        plugin->nullRenderInit();
    }
    forEachPlugin(i, plugin) {
        executeMultipleJobs(plugin, plugin->postBackgroundRenderActions());
    }
}

// Perform extra rendering jobs from plugins.
void OpenGLScreen::renderExtraJobs() {
    OpenGLPlugin *plugin = NULL;

    forEachPlugin(i, plugin) {
        plugin->nullRenderInit();
    }
    forEachPlugin(i, plugin) {
        executeMultipleJobs(plugin, plugin->extraRenderingActions());
        plugin->postExtraRenderingActions();
    }
}

// Render the reconfigure rectangle.
void OpenGLScreen::renderReconfigureRect() {
    OpenGLPlugin *plugin = NULL;

    // Convert reconfigure rectangle to OpenGL coordinates.
    m_rec_rect_line_pos_buffer->bufferPosRectangle(rootWindow().width(), rootWindow().height(), reconfigureRectangle());

    // Render it.
    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_XOR);

    forEachPlugin(i, plugin) {
        plugin->recRectRenderInit(reconfigureRectangle());
    }
    render(GL_LINE_STRIP, m_rec_rect_line_pos_buffer, m_default_tex_coord_buffer, m_white_texture,
           m_default_tex_coord_buffer, m_white_texture, m_rec_rect_element_buffer, 5, 1.0);
    forEachPlugin(i, plugin) {
        plugin->recRectRenderCleanup(reconfigureRectangle());
    }

    glDisable(GL_COLOR_LOGIC_OP);
}

// A function to render a particular window onto the screen.
void OpenGLScreen::renderWindow(OpenGLWindow &window) {
    OpenGLPlugin *plugin = NULL;

    // Update window's contents.
    if (window.isDamaged()) {
        window.updateContents();
    }

    // Extra rendering jobs before a window is drawn.
    forEachPlugin(i, plugin) {
        plugin->nullRenderInit();
    }
    forEachPlugin(i, plugin) {
        executeMultipleJobs(plugin, plugin->preWindowRenderActions(window));
    }

    // Render it.
    for (int i = 0; i < window.partitionCount(); i++) {
        forEachPlugin(j, plugin) {
            plugin->windowRenderInit(window, i);
        }
        render(GL_TRIANGLE_STRIP, window.partitionPosBuffer(i),
               m_default_tex_coord_buffer, window.contentTexturePartition(i),
               m_default_tex_coord_buffer, window.shapeTexturePartition(i),
               m_default_element_buffer, 4, window.alpha() / 255.0);
        forEachPlugin(j, plugin) {
            plugin->windowRenderCleanup(window, i);
        }
    }

    // Extra rendering jobs after a window is drawn.
    forEachPlugin(i, plugin) {
        plugin->nullRenderInit();
    }
    forEachPlugin(i, plugin) {
        executeMultipleJobs(plugin, plugin->postWindowRenderActions(window));
    }
}


// Execute multiple rendering jobs.
void OpenGLScreen::executeMultipleJobs(OpenGLPlugin *plugin, const std::vector<OpenGLRenderingJob> &jobs) {
    for (size_t i = 0; i < jobs.size(); i++) {
        executeRenderingJob(jobs[i]);
    }
    plugin->nullRenderInit();
}

// Execute a given rendering job.
void OpenGLScreen::executeRenderingJob(const OpenGLRenderingJob &job) {
    if ((job.alpha >= 0.0) && (job.alpha <= 1.0)) {
        job.shader_init->execute();
        render(GL_TRIANGLE_STRIP, job.prim_pos_buffer, job.main_tex_coord_buffer, job.main_texture,
               job.shape_tex_coord_buffer, job.shape_texture, m_default_element_buffer, 4, job.alpha);
        job.shader_deinit->execute();
    }
}

// A function to render something onto the screen.
void OpenGLScreen::render(GLenum rendering_mode, OpenGLBufferPtr prim_pos_buffer,
                          OpenGLBufferPtr main_tex_coord_buffer, OpenGL2DTexturePtr main_texture,
                          OpenGLBufferPtr shape_tex_coord_buffer, OpenGL2DTexturePtr shape_texture,
                          OpenGLBufferPtr element_buffer, GLuint element_count, GLfloat alpha) {
    // Load primitive position vertex array.
    glBindBuffer(GL_ARRAY_BUFFER, prim_pos_buffer->handle());
    glVertexAttribPointer(m_shader_program->primPosAttrib(), 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(m_shader_program->primPosAttrib());

    // Load main texture position vertex array.
    glBindBuffer(GL_ARRAY_BUFFER, main_tex_coord_buffer->handle());
    glVertexAttribPointer(m_shader_program->mainTexCoordAttrib(), 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(m_shader_program->mainTexCoordAttrib());

    // Load shape texture position vertex array.
    glBindBuffer(GL_ARRAY_BUFFER, shape_tex_coord_buffer->handle());
    glVertexAttribPointer(m_shader_program->shapeTexCoordAttrib(), 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)(0));
    glEnableVertexAttribArray(m_shader_program->shapeTexCoordAttrib());

    // Set up textures.
    glUniform1i(m_shader_program->mainTexUniform(), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, main_texture->handle());

    glUniform1i(m_shader_program->shapeTexUniform(), 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shape_texture->handle());

    // Set up other uniforms.
    glUniform1f(m_shader_program->alphaUniform(), alpha);

    // Load element array.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer->handle());

    // Final setup.
    if (m_have_double_buffering) {
        glDrawBuffer(GL_BACK);
    }
    glViewport(0, 0, rootWindow().width(), rootWindow().height());

    // Render!
    glDrawElements(rendering_mode, element_count, GL_UNSIGNED_SHORT, (void*)0);

    // Cleanup.
    glDisableVertexAttribArray(m_shader_program->mainTexCoordAttrib());
    glDisableVertexAttribArray(m_shader_program->primPosAttrib());
    glDisableVertexAttribArray(m_shader_program->shapeTexCoordAttrib());
}
