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
#include "Utility.hh"
#include "XRenderScreen.hh"
#include "XRenderWindow.hh"

#include <algorithm>
#include <iostream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
FadePlugin::FadePlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) :
    XRenderPlugin(screen, args) {

    m_fade_pict_format = XRenderFindStandardFormat(display(), PictStandardARGB32);
}

// Destructor.
FadePlugin::~FadePlugin() { }


//--- WINDOW EVENT CALLBACKS ---------------------------------------------------

// Called, whenever a window becomes ignored.
void FadePlugin::windowBecameIgnored(const BaseCompWindow &window) {
    std::map<Window, PosFadeData>::iterator pos_it = m_positive_fades.find(window.window());
    if (pos_it != m_positive_fades.end()) {
        m_positive_fades.erase(pos_it);
    } 

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
    PosFadeData fade;

    std::vector<NegFadeData>::iterator it = m_negative_fades.begin();
    while (true) {
        if (it == m_negative_fades.end()) {
            fade.fade_alpha = 0;
            fade.fade_picture = new XRenderPicture(xrenderScreen(), m_fade_pict_format, xrenderScreen().pictFilter());
            break;
        } else if (it->window_id == window.window()) {
            fade.fade_alpha = it->fade_alpha;
            fade.fade_picture = it->fade_picture;
            m_negative_fades.erase(it);
            break;
        } else {
            ++it;
        }
    }

    fade.dimensions = window.dimensions();
    fade.timer.setTickSize(250000 / 255);
    fade.timer.start();

    m_positive_fades.insert(std::make_pair(window.window(), fade));
}

// Called, whenever a window is unmapped.
void FadePlugin::windowUnmapped(const BaseCompWindow &window) {
    const XRenderWindow &xr_window = dynamic_cast<const XRenderWindow&>(window);
    NegFadeData fade;

    std::map<Window, PosFadeData>::iterator it = m_positive_fades.find(window.window());
    if (it != m_positive_fades.end()) {
        fade.fade_alpha = it->second.fade_alpha;
        fade.fade_picture = it->second.fade_picture;
        m_positive_fades.erase(it);
    } else {
        fade.fade_alpha = 255;
        fade.fade_picture = new XRenderPicture(xrenderScreen(), m_fade_pict_format, xrenderScreen().pictFilter());
    }

    if (xr_window.contentPicture()->pictureHandle() != None) {
        fade.dimensions = xr_window.dimensions();
        fade.mask_picture = xr_window.maskPicture();
        fade.window_id = xr_window.window();

        fade.job.operation = PictOpOver;
        fade.job.source_picture = xr_window.contentPicture();
        fade.job.source_x = 0;
        fade.job.source_y = 0;
        fade.job.mask_x = 0;
        fade.job.mask_y = 0;
        fade.job.destination_x = xr_window.x();
        fade.job.destination_y = xr_window.y();
        fade.job.width = xr_window.realWidth();
        fade.job.height = xr_window.realHeight();

        fade.timer.setTickSize(250000 / 255);
        fade.timer.start();

        m_negative_fades.push_back(fade);
    }
}


//--- RENDERING ACTIONS --------------------------------------------------------

// Rectangles that the plugin wishes to damage.
const std::vector<XRectangle> &FadePlugin::damagedAreas() {
    m_damaged_areas.clear();     // TODO: Stop recreating vector's contents.

    std::map<Window, PosFadeData>::iterator pos_it = m_positive_fades.begin();
    while (pos_it != m_positive_fades.end()) {
        m_damaged_areas.push_back(pos_it->second.dimensions);
        ++pos_it;
    }

    std::vector<NegFadeData>::iterator neg_it = m_negative_fades.begin();
    while (neg_it != m_negative_fades.end()) {
        m_damaged_areas.push_back(neg_it->dimensions);
        ++neg_it;
    }

    return m_damaged_areas;
}


// Window rendering job initialization.
void FadePlugin::windowRenderingJobInit(const XRenderWindow &window, XRenderRenderingJob &job) {
    std::map<Window, PosFadeData>::iterator it = m_positive_fades.find(window.window());
    if (it != m_positive_fades.end()) {
        PosFadeData &curFade = it->second;

        int new_ticks;
        try {
            new_ticks = curFade.timer.newElapsedTicks();
        } catch (const TimeException &e) {
            new_ticks = 255;
        }

        if ((new_ticks > 0) || (curFade.fade_picture->pictureHandle() == None)) {
            curFade.fade_alpha += new_ticks;
            if (curFade.fade_alpha > 255) {
                curFade.fade_alpha = 255;
            }

            createFadedMask(curFade.fade_alpha, window.maskPicture(), window.dimensions(), curFade.fade_picture);
        }

        if (curFade.fade_picture->pictureHandle() != None) {
            job.mask_picture = curFade.fade_picture;
        }
    }
}

// Extra rendering actions and jobs.
const std::vector<XRenderRenderingJob> &FadePlugin::extraRenderingActions() {
    m_extra_jobs.clear();    // TODO: Stop recreating vector's contents.

    for (size_t i = 0; i < m_negative_fades.size(); i++) {
        int new_ticks;
        try {
            new_ticks = m_negative_fades[i].timer.newElapsedTicks();
        } catch (const TimeException &e) {
            new_ticks = 255;
        }

        if ((new_ticks > 0) || (m_negative_fades[i].fade_picture->pictureHandle() == None)) {
            m_negative_fades[i].fade_alpha -= new_ticks;
            if (m_negative_fades[i].fade_alpha < 0) {
                m_negative_fades[i].fade_alpha = 0;
            }

            createFadedMask(m_negative_fades[i].fade_alpha, m_negative_fades[i].mask_picture,
                            m_negative_fades[i].dimensions, m_negative_fades[i].fade_picture);
        }

        if (m_negative_fades[i].fade_picture->pictureHandle() != None) {
            m_negative_fades[i].job.mask_picture = m_negative_fades[i].fade_picture;
            m_extra_jobs.push_back(m_negative_fades[i].job);
        }
    }

    return m_extra_jobs;
}

// Called after the extra rendering jobs are executed.
void FadePlugin::postExtraRenderingActions() {
    std::map<Window, PosFadeData>::iterator pos_it = m_positive_fades.begin();
    while (pos_it != m_positive_fades.end()) {
        if (pos_it->second.fade_alpha >= 255) {
            m_positive_fades.erase(pos_it++);
        } else {
            ++pos_it;
        }
    }

    std::vector<NegFadeData>::iterator neg_it = m_negative_fades.begin();
    while (neg_it != m_negative_fades.end()) {
        if (neg_it->fade_alpha <= 0) {
            neg_it = m_negative_fades.erase(neg_it);
        } else {
            ++neg_it;
        }
    }
}


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Returns the faded mask picture for the given window fade.
void FadePlugin::createFadedMask(int alpha, XRenderPicturePtr mask, XRectangle dimensions,
                                 XRenderPicturePtr fade_picture_return) {
    if (mask->pictureHandle() == None) {
        return;
    }

    Pixmap fadePixmap = createSolidPixmap(screen(), dimensions.width, dimensions.height, alpha * 0x01010101);
    fade_picture_return->setPixmap(fadePixmap, true);

    XRenderComposite(display(), PictOpIn, mask->pictureHandle(), None, fade_picture_return->pictureHandle(),
                     0, 0, 0, 0, 0, 0, dimensions.width, dimensions.height);
}


//--- PLUGIN MANAGER FUNCTIONS -------------------------------------------------

// Creates a plugin object.
extern "C" BasePlugin *createPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) {
    return new FadePlugin(screen, args);
}

// Returns the type of the plugin.
extern "C" FbCompositor::PluginType pluginType() {
    return Plugin_XRender;
}
