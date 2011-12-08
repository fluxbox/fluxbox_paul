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
#include "Utility.hh"
#include "XRenderScreen.hh"
#include "XRenderWindow.hh"

#include <algorithm>
#include <iostream>

using namespace FbCompositor;


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
    XRenderPlugin(screen, args) {

    unsigned long mask_color = 0x01010101 * PREVIEW_ALPHA;
    Pixmap mask_pixmap = createSolidPixmap(screen, MAX_PREVIEW_WIDTH, MAX_PREVIEW_HEIGHT, mask_color);
    XRenderPictFormat *pict_format = XRenderFindStandardFormat(display(), PictStandardARGB32);
    m_mask_picture = new XRenderPicture(xrenderScreen(), pict_format, FilterFast);
    m_mask_picture->setPixmap(mask_pixmap, true);

    m_previous_damage.width = 0;
    m_previous_damage.height = 0;
    m_previous_window = None;

    m_tick_tracker.setTickSize(SLEEP_TIME);
}

// Destructor.
PreviewPlugin::~PreviewPlugin() { }



//--- WINDOW EVENT CALLBACKS ---------------------------------------------------

// Called, whenever a new window is created.
void PreviewPlugin::windowCreated(const BaseCompWindow &window) {
    const XRenderWindow &xr_window = dynamic_cast<const XRenderWindow&>(window);

    XRenderPictFormat *pict_format = XRenderFindStandardFormat(display(), PictStandardARGB32);
    XRenderPicturePtr thumbnail(new XRenderPicture(xrenderScreen(), pict_format, FilterBest));

    Pixmap thumb_pixmap = createSolidPixmap(screen(), MAX_PREVIEW_WIDTH, MAX_PREVIEW_HEIGHT);
    thumbnail->setPixmap(thumb_pixmap, true);

    XRenderRenderingJob job = { PictOpOver, thumbnail, m_mask_picture, 0, 0, 0, 0, 0, 0, 0, 0 };

    PreviewWindowData win_data = { xr_window, job };
    m_preview_data.insert(std::make_pair(xr_window.window(), win_data));
}

/** Called, whenever a window is destroyed. */
void PreviewPlugin::windowDestroyed(const BaseCompWindow &window) {
    std::map<Window, PreviewWindowData>::iterator it = m_preview_data.find(window.window());
    if (it != m_preview_data.end()) {
        m_preview_data.erase(it);
    }
}


//--- RENDERING ACTIONS --------------------------------------------------------

/** Rectangles that the plugin wishes to damage. */
const std::vector<XRectangle> &PreviewPlugin::damagedAreas() {
    m_damaged_areas.clear();

    if ((m_previous_damage.width > 0) && (m_previous_damage.height > 0)) {
        m_damaged_areas.push_back(m_previous_damage);
    }

    std::map<Window, PreviewWindowData>::iterator it = m_preview_data.find(screen().currentIconbarItem());
    if (it != m_preview_data.end()) {
        Window cur_window = it->first;
        PreviewWindowData &cur_preview = it->second;

        if ((m_previous_window != cur_window)
                && (cur_preview.window.contentPicture()->pictureHandle())
                && (cur_preview.window.maskPicture()->pictureHandle())) {
            m_previous_window = cur_window;
            updatePreviewWindowData(cur_preview);
        }

        updatePreviewWindowPos(cur_preview);

        XRectangle cur_damage = { cur_preview.job.destination_x, cur_preview.job.destination_y,
                                  cur_preview.job.width, cur_preview.job.height };
        m_damaged_areas.push_back(cur_damage);
        m_previous_damage = cur_damage;

        if (!m_tick_tracker.isRunning()) {
            m_tick_tracker.start();
        }
    } else {
        m_previous_damage.width = 0;
        m_previous_damage.height = 0;
        m_previous_window = None;
        m_tick_tracker.stop();
    }

    return m_damaged_areas;
}

/** Extra rendering actions and jobs. */
const std::vector<XRenderRenderingJob> &PreviewPlugin::extraRenderingActions() {
    m_extra_jobs.clear();

    std::map<Window, PreviewWindowData>::iterator it = m_preview_data.find(xrenderScreen().currentIconbarItem());
    if (it != m_preview_data.end()) {
        PreviewWindowData &cur_preview = it->second;

        if ((cur_preview.job.source_picture->pictureHandle()) && (m_tick_tracker.totalElapsedTicks() > 0)) {
            m_extra_jobs.push_back(cur_preview.job);
        }
    }

    return m_extra_jobs;
}


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Update the preview window data.
void PreviewPlugin::updatePreviewWindowData(PreviewWindowData &win_preview) {
    double scale_factor = 1.0;
    scale_factor = std::max(scale_factor, win_preview.window.realWidth() / double(MAX_PREVIEW_WIDTH));
    scale_factor = std::max(scale_factor, win_preview.window.realHeight() / double(MAX_PREVIEW_HEIGHT));

    int thumbWidth = static_cast<int>(win_preview.window.realWidth() / scale_factor);
    int thumbHeight = static_cast<int>(win_preview.window.realHeight() / scale_factor);

    win_preview.window.contentPicture()->scalePicture(scale_factor, scale_factor);
    win_preview.window.maskPicture()->scalePicture(scale_factor, scale_factor);

    XRenderComposite(display(), PictOpSrc,
                     win_preview.window.contentPicture()->pictureHandle(), 
                     win_preview.window.maskPicture()->pictureHandle(),
                     win_preview.job.source_picture->pictureHandle(),
                     0, 0, 0, 0, 0, 0, thumbWidth, thumbHeight);

    win_preview.window.contentPicture()->resetPictureTransform();
    win_preview.window.maskPicture()->resetPictureTransform();

    win_preview.job.width = thumbWidth;
    win_preview.job.height = thumbHeight;
}

// Update the preview window position.
// TODO: Place the preview window on the edge of the toolbar.
// TODO: Left/Right toolbar orientations.
void PreviewPlugin::updatePreviewWindowPos(PreviewWindowData &win_preview) {
    int mouse_pos_x, mouse_pos_y;
    mousePointerLocation(screen(), mouse_pos_x, mouse_pos_y);

    if (screen().heads().size() > 0) {
        XRectangle cur_head = screen().heads()[0];

        for (size_t i = 1; i < screen().heads().size(); i++) {
            XRectangle head = screen().heads()[i];
            if ((mouse_pos_x >= head.x) && (mouse_pos_y >= head.y)
                    && (mouse_pos_x < (head.x + head.width))
                    && (mouse_pos_y < (head.y + head.height))) {
                cur_head = head;
                break;
            }
        }

        win_preview.job.destination_x = mouse_pos_x - (win_preview.job.width / 2);

        int mid_head = cur_head.y + (cur_head.height / 2);
        if (mouse_pos_y < mid_head) {
            win_preview.job.destination_y = mouse_pos_y + 10;
        } else {
            win_preview.job.destination_y = mouse_pos_y - win_preview.job.height - 10;
        }
    } else {    // But what IF.
        win_preview.job.destination_x = mouse_pos_x - (win_preview.job.width / 2);
        win_preview.job.destination_y = mouse_pos_y + 10;
    }
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

// Creates a plugin object.
extern "C" BasePlugin *createPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) {
    return new PreviewPlugin(screen, args);
}

// Returns the type of the plugin.
extern "C" FbCompositor::PluginType pluginType() {
    return Plugin_XRender;
}
