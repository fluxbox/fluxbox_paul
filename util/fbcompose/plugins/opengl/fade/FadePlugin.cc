/** FadePlugin.cc file for the fluxbox compositor. */

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


#include "FadePlugin.hh"

#include "BaseScreen.hh"
#include "Exceptions.hh"
#include "OpenGLScreen.hh"

#include <algorithm>
#include <iostream>

using namespace FbCompositor;


//--- SHADER SOURCES -----------------------------------------------------------

/** Plugin's fragment shader source. */
const GLchar FRAGMENT_SHADER[] = "\
    uniform float fade_Alpha;                                                \n\
                                                                             \n\
    void fade() {                                                            \n\
        gl_FragColor *= vec4(1.0, 1.0, 1.0, fade_Alpha);                     \n\
    }                                                                        \n\
";

/** Plugin's vertex shader source. */
const GLchar VERTEX_SHADER[] = "\
    void fade() { }                                                          \n\
";


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
FadePlugin::FadePlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) :
    OpenGLPlugin(screen, args),
    m_shader_initializer() {
}

// Destructor.
FadePlugin::~FadePlugin() { }


//--- OTHER INITIALIZATION -----------------------------------------------------

// Initialize OpenGL-specific code.
void FadePlugin::initOpenGL(OpenGLShaderProgramPtr shader_program) {
    m_alpha_uniform_pos = shader_program->getUniformLocation("fade_Alpha");
    m_shader_initializer.setUniform(m_alpha_uniform_pos);
}


//--- ACCESSORS ----------------------------------------------------------------

// Returns the additional source code for the fragment shader.
const char *FadePlugin::fragmentShader() const {
    return FRAGMENT_SHADER;
}

// Returns the additional source code for the vertex shader.
const char *FadePlugin::vertexShader() const {
    return VERTEX_SHADER;
}


//--- WINDOW EVENT CALLBACKS ---------------------------------------------------

// Called, whenever a window becomes ignored.
void FadePlugin::windowBecameIgnored(const BaseCompWindow &window) {
    // Remove the window's positive fade, if any.
    std::map<Window, PosFadeData>::iterator pos_it = m_positive_fades.find(window.window());
    if (pos_it != m_positive_fades.end()) {
        m_positive_fades.erase(pos_it);
    } 

    // Remove the window's negative fade, if any.
    std::vector<NegFadeData>::iterator neg_it = m_negative_fades.begin();
    while (neg_it != m_negative_fades.end()) {
        if (neg_it->window_id == window.window()) {
            m_negative_fades.erase(neg_it);
            break;
        } 
        ++neg_it;
    }
}

// Called, whenever a window is mapped.
void FadePlugin::windowMapped(const BaseCompWindow &window) {
    PosFadeData new_fade;

    // Is the window being faded out?
    std::vector<NegFadeData>::iterator it = m_negative_fades.begin();
    while (true) {
        if (it == m_negative_fades.end()) {
            new_fade.fade_alpha = 0;
            break;
        } else if (it->window_id == window.window()) {
            new_fade.fade_alpha = it->fade_alpha;
            m_negative_fades.erase(it);
            break;
        } else {
            ++it;
        }
    }

    // Initialize the remaining fields.
    new_fade.timer.setTickSize(250000 / 255);
    new_fade.timer.start();

    // Track the fade.
    m_positive_fades.insert(std::make_pair(window.window(), new_fade));
}

// Called, whenever a window is unmapped.
void FadePlugin::windowUnmapped(const BaseCompWindow &window) {
    const OpenGLWindow &gl_window = dynamic_cast<const OpenGLWindow&>(window);

    // Is the window being faded in?
    float fade_alpha = 255;
    std::map<Window, PosFadeData>::iterator it = m_positive_fades.find(window.window());

    if (it != m_positive_fades.end()) {
        fade_alpha = it->second.fade_alpha;
        m_positive_fades.erase(it);
    }

    // Create a fade for each window partition.
    for (int i = 0; i < gl_window.partitionCount(); i++) {
        NegFadeData new_fade;

        new_fade.fade_alpha = fade_alpha;
        new_fade.timer.setTickSize(250000 / 255);
        new_fade.timer.start();
        new_fade.window_id = gl_window.window();

        new_fade.job.prim_pos_buffer = gl_window.partitionPosBuffer(i);
        new_fade.job.main_tex_coord_buffer = openGLScreen().defaultTexCoordBuffer();
        new_fade.job.main_texture = gl_window.contentTexturePartition(i);
        new_fade.job.shape_tex_coord_buffer = openGLScreen().defaultTexCoordBuffer();
        new_fade.job.shape_texture = gl_window.shapeTexturePartition(i);
        new_fade.job.alpha = gl_window.alpha() / 255.0;

        new_fade.job.shader_init = new FadeShaderInitializer(m_alpha_uniform_pos, 0.0);
        new_fade.job.shader_deinit = new NullDeinitializer();

        m_negative_fades.push_back(new_fade);
    }
}


//--- RENDERING ACTIONS --------------------------------------------------------

// Background rendering initialization.
void FadePlugin::backgroundRenderInit(int /*part_id*/) {
    m_shader_initializer.setAlpha(1.0);
    m_shader_initializer.execute();
}

// Window rendering initialization.
void FadePlugin::windowRenderInit(const OpenGLWindow &window, int /*part_id*/) {
    std::map<Window, PosFadeData>::iterator it = m_positive_fades.find(window.window());
    if (it != m_positive_fades.end()) {
        try {
            it->second.fade_alpha += it->second.timer.newElapsedTicks();
        } catch (const TimeException &e) {
            it->second.fade_alpha = 255;
        }
        
        if (it->second.fade_alpha >= 255) {
            m_shader_initializer.setAlpha(1.0);
            m_positive_fades.erase(it);
        } else {
            m_shader_initializer.setAlpha(it->second.fade_alpha / 255.0);
        }
    } else {
        m_shader_initializer.setAlpha(1.0);
    }

    m_shader_initializer.execute();
}

// Reconfigure rectangle rendering initialization.
void FadePlugin::recRectRenderInit(const XRectangle &/*rec_rect*/) {
    m_shader_initializer.setAlpha(1.0);
    m_shader_initializer.execute();
}


// Extra rendering actions and jobs.
const std::vector<OpenGLRenderingJob> &FadePlugin::extraRenderingActions() {
    m_extra_jobs.clear();    // TODO: Stop copying jobs on every call.

    for (size_t i = 0; i < m_negative_fades.size(); i++) {
        try {
            m_negative_fades[i].fade_alpha -= m_negative_fades[i].timer.newElapsedTicks();
        } catch (const TimeException &e) {
            m_negative_fades[i].fade_alpha = 0;
        }

        if (m_negative_fades[i].fade_alpha <= 0) {
            m_negative_fades[i].fade_alpha = 0;
        }

        (dynamic_cast<FadeShaderInitializer*>(m_negative_fades[i].job.shader_init))->setAlpha(m_negative_fades[i].fade_alpha / 255.0);
        m_extra_jobs.push_back(m_negative_fades[i].job);
    }

    return m_extra_jobs;
}

// Post extra rendering actions.
void FadePlugin::postExtraRenderingActions() {
    std::vector<NegFadeData>::iterator it = m_negative_fades.begin();
    while (it != m_negative_fades.end()) {
        if (it->fade_alpha <= 0) {
            delete it->job.shader_init;
            delete it->job.shader_deinit;
            it = m_negative_fades.erase(it);
        } else {
            ++it;
        }
    }
}


// Null rendering job initialization.
void FadePlugin::nullRenderInit() {
    m_shader_initializer.setAlpha(1.0);
    m_shader_initializer.execute();
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

// Creates a plugin object.
extern "C" BasePlugin *createPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) {
    return new FadePlugin(screen, args);
}

// Returns the type of the plugin.
extern "C" FbCompositor::PluginType pluginType() {
    return Plugin_OpenGL;
}
