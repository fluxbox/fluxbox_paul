/** PreviewPlugin.cc file for the fluxbox compositor. */

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


#include "PreviewPlugin.hh"

#include "BaseScreen.hh"
#include "Exceptions.hh"
#include "OpenGLScreen.hh"
#include "OpenGLWindow.hh"
#include "Utility.hh"

#include <algorithm>
#include <iostream>

using namespace FbCompositor;


//--- SHADER SOURCES -----------------------------------------------------------

/** Plugin's fragment shader source. */
const GLchar FRAGMENT_SHADER[] = "\
    void preview() { }                                                       \n\
";

/** Plugin's vertex shader source. */
const GLchar VERTEX_SHADER[] = "\
    void preview() { }                                                       \n\
";


//--- CONSTANTS ----------------------------------------------------------------

/** Maximum height of the preview window. */
const int MAX_PREVIEW_HEIGHT = 150;

/** Maximum width of the preview window. */
const int MAX_PREVIEW_WIDTH = 150;

/** Transparency of the preview window. */
const unsigned int PREVIEW_ALPHA = 200;

/** Time in microseconds until the preview window is shown. */
const int SLEEP_TIME = 500000;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
PreviewPlugin::PreviewPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) :
    OpenGLPlugin(screen, args) {

    m_tick_tracker.setTickSize(SLEEP_TIME);
}

// Destructor.
PreviewPlugin::~PreviewPlugin() { }


//--- ACCESSORS ----------------------------------------------------------------

// Returns the additional source code for the fragment shader.
const char *PreviewPlugin::fragmentShader() const {
    return FRAGMENT_SHADER;
}

// Returns the additional source code for the vertex shader.
const char *PreviewPlugin::vertexShader() const {
    return VERTEX_SHADER;
}


//--- WINDOW EVENT CALLBACKS ---------------------------------------------------

// Called, whenever a new window is created.
void PreviewPlugin::windowCreated(const BaseCompWindow &window) {
    const OpenGLWindow &gl_window = dynamic_cast<const OpenGLWindow&>(window);

    OpenGLRenderingJob job;
    job.prim_pos_buffer = new OpenGLBuffer(openGLScreen(), GL_ARRAY_BUFFER);
    job.main_tex_coord_buffer = openGLScreen().defaultTexCoordBuffer();
    job.shape_tex_coord_buffer = openGLScreen().defaultTexCoordBuffer();
    job.alpha = PREVIEW_ALPHA / 255.0f;
    job.shader_init = new NullInitializer();
    job.shader_deinit = new NullDeinitializer();

    PreviewWindowData win_data = { gl_window, job };
    m_preview_data.insert(std::make_pair(gl_window.window(), win_data));
}

/** Called, whenever a window is destroyed. */
void PreviewPlugin::windowDestroyed(const BaseCompWindow &window) {
    std::map<Window, PreviewWindowData>::iterator it = m_preview_data.find(window.window());
    if (it != m_preview_data.end()) {
        m_preview_data.erase(it);
    }
}


//--- RENDERING ACTIONS --------------------------------------------------------

/** Extra rendering actions and jobs. */
const std::vector<OpenGLRenderingJob> &PreviewPlugin::extraRenderingActions() {
    m_extra_jobs.clear();

    std::map<Window, PreviewWindowData>::iterator it = m_preview_data.find(screen().currentIconbarItem());
    if (it != m_preview_data.end()) {
        PreviewWindowData &cur_preview = it->second;

        if (!m_tick_tracker.isRunning()) {
            m_tick_tracker.start();
        }

        if (cur_preview.window.partitionCount() > 0) {
            updatePreviewWindow(cur_preview);
            if (m_tick_tracker.totalElapsedTicks() > 0) {
                m_extra_jobs.push_back(cur_preview.job);
            }
        }
    } else {
        m_tick_tracker.stop();
    }

    return m_extra_jobs;
}


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Update the preview window.
// TODO: Place the preview window on the edge of the toolbar.
// TODO: Left/Right toolbar orientations.
// TODO: Use all window texture partitions.
void PreviewPlugin::updatePreviewWindow(PreviewWindowData &win_preview) {
    XRectangle thumb_dim;

    // Find thumbnail's width and height.
    int full_thumb_width = std::min<int>(win_preview.window.realWidth(), openGLScreen().maxTextureSize());
    int full_thumb_height = std::min<int>(win_preview.window.realHeight(), openGLScreen().maxTextureSize());

    double scale_factor = 1.0;
    scale_factor = std::max(scale_factor, full_thumb_width / double(MAX_PREVIEW_WIDTH));
    scale_factor = std::max(scale_factor, full_thumb_height / double(MAX_PREVIEW_HEIGHT));

    thumb_dim.width = static_cast<int>(full_thumb_width / scale_factor);
    thumb_dim.height = static_cast<int>(full_thumb_height / scale_factor);

    // Find thumbnail's X and Y coordinates.
    int mouse_pos_x, mouse_pos_y;
    mousePointerLocation(screen(), mouse_pos_x, mouse_pos_y);

    if (screen().heads().size() > 0) {
        XRectangle cur_head = screen().heads()[0];

        // First find which head the mouse pointer is on,
        for (size_t i = 1; i < screen().heads().size(); i++) {
            XRectangle head = screen().heads()[i];
            if ((mouse_pos_x >= head.x) && (mouse_pos_y >= head.y)
                    && (mouse_pos_x < (head.x + head.width))
                    && (mouse_pos_y < (head.y + head.height))) {
                cur_head = head;
                break;
            }
        }

        // then position the thumbnail there.
        thumb_dim.x = mouse_pos_x - thumb_dim.width / 2;

        int mid_head = cur_head.y + (cur_head.height / 2);
        if (mouse_pos_y < mid_head) {
            thumb_dim.y = mouse_pos_y + 10;
        } else {
            thumb_dim.y = mouse_pos_y - thumb_dim.height - 10;
        }
    } else {    // But WHAT IF we have no heads?
        thumb_dim.x = mouse_pos_x - (thumb_dim.width / 2);
        thumb_dim.y = mouse_pos_y + 10;
    }

    // Update job struct variables.
    win_preview.job.prim_pos_buffer->bufferPosRectangle(screen().rootWindow().width(), screen().rootWindow().height(), thumb_dim);

    win_preview.job.main_texture = win_preview.window.contentTexturePartition(0);
    win_preview.job.shape_texture = win_preview.window.shapeTexturePartition(0);
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

// Creates a plugin object.
extern "C" BasePlugin *createPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) {
    return new PreviewPlugin(screen, args);
}

// Returns the type of the plugin.
extern "C" FbCompositor::PluginType pluginType() {
    return Plugin_OpenGL;
}
