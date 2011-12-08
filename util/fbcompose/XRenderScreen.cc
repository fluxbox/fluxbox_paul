/** XRenderScreen.cc file for the fluxbox compositor. */

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

#include "XRenderScreen.hh"

#include "CompositorConfig.hh"
#include "Logging.hh"
#include "Utility.hh"
#include "XRenderWindow.hh"

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xutil.h>

using namespace FbCompositor;


//--- MACROS -------------------------------------------------------------------

// Macro for plugin iteration.
#define forEachPlugin(i, plugin)                                                        \
    (plugin) = ((pluginManager().plugins().size() > 0)                                  \
                   ? (dynamic_cast<XRenderPlugin*>(pluginManager().plugins()[0]))       \
                   : NULL);                                                             \
    for(size_t (i) = 0;                                                                 \
        ((i) < pluginManager().plugins().size());                                       \
        (i)++,                                                                          \
        (plugin) = (((i) < pluginManager().plugins().size())                            \
                       ? (dynamic_cast<XRenderPlugin*>(pluginManager().plugins()[(i)])) \
                       : NULL))


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
XRenderScreen::XRenderScreen(int screen_number, const CompositorConfig &config):
    BaseScreen(screen_number, Plugin_XRender, config),
    m_pict_filter(config.xRenderPictFilter()) {

    m_plugin_damage = XFixesCreateRegion(display(), NULL, 0);

    initRenderingSurface();
    updateBackgroundPicture();
}

// Destructor.
XRenderScreen::~XRenderScreen() {
    if (m_plugin_damage) {
        XFixesDestroyRegion(display(), m_plugin_damage);
    }

    XUnmapWindow(display(), m_rendering_window);
    XDestroyWindow(display(), m_rendering_window);
}


//--- INITIALIZATION FUNCTIONS -------------------------------------------------

// Initializes the rendering surface.
void XRenderScreen::initRenderingSurface() {
    // Get all the elements, needed for the creation of the rendering surface.
    Window comp_overlay = XCompositeGetOverlayWindow(display(), rootWindow().window());

    XVisualInfo visual_info;
    if (!XMatchVisualInfo(display(), screenNumber(), 32, TrueColor, &visual_info)) {
        throw InitException("Cannot find the required visual.");
    }

    XSetWindowAttributes wa;
    wa.border_pixel = XBlackPixel(display(), screenNumber());   // Without this XCreateWindow gives BadMatch error.
    wa.colormap = XCreateColormap(display(), rootWindow().window(), visual_info.visual, AllocNone);
    long wa_mask = CWBorderPixel | CWColormap;

    // Create the rendering surface.
    m_rendering_window = XCreateWindow(display(), comp_overlay, 0, 0, rootWindow().width(), rootWindow().height(), 0,
                                      visual_info.depth, InputOutput, visual_info.visual, wa_mask, &wa);
    XmbSetWMProperties(display(), m_rendering_window, "fbcompose", "fbcompose", NULL, 0, NULL, NULL, NULL);
    XMapWindow(display(), m_rendering_window);

    // Make sure the overlays do not consume any input events.
    XserverRegion empty_region = XFixesCreateRegion(display(), NULL, 0);
    XFixesSetWindowShapeRegion(display(), comp_overlay, ShapeInput, 0, 0, empty_region);
    XFixesSetWindowShapeRegion(display(), m_rendering_window, ShapeInput, 0, 0, empty_region);
    XFixesDestroyRegion(display(), empty_region);

    ignoreWindow(comp_overlay);
    ignoreWindow(m_rendering_window);

    // Create an XRender picture for the rendering window.
    XRenderPictureAttributes pa;
    pa.subwindow_mode = IncludeInferiors;
    long pa_mask = CPSubwindowMode;

    XRenderPictFormat *rendering_pict_format = XRenderFindVisualFormat(display(), visual_info.visual);
    if (!rendering_pict_format) {
        throw InitException("Cannot find the required picture format.");
    }

    m_rendering_picture = new XRenderPicture(*this, rendering_pict_format, m_pict_filter);
    m_rendering_picture->setWindow(m_rendering_window, pa, pa_mask);

    // Create the back buffer.
    XRenderPictFormat *back_buffer_pict_format = XRenderFindStandardFormat(display(), PictStandardARGB32);
    Pixmap back_buffer_pixmap = XCreatePixmap(display(), rootWindow().window(), rootWindow().width(), rootWindow().height(), 32);

    m_back_buffer_picture = new XRenderPicture(*this, back_buffer_pict_format, m_pict_filter);
    m_back_buffer_picture->setPixmap(back_buffer_pixmap, true, pa, pa_mask);
}


//--- SCREEN MANIPULATION ------------------------------------------------------

// Notifies the screen of a background change.
void XRenderScreen::setRootPixmapChanged() {
    BaseScreen::setRootPixmapChanged();
    m_root_changed = true;
}

// Notifies the screen of a root window change.
void XRenderScreen::setRootWindowSizeChanged() {
    BaseScreen::setRootWindowSizeChanged();
    m_root_changed = true;

    XRenderPictureAttributes pa;
    pa.subwindow_mode = IncludeInferiors;
    long pa_mask = CPSubwindowMode;

    XResizeWindow(display(), m_rendering_window, rootWindow().width(), rootWindow().height());
    m_rendering_picture->setWindow(m_rendering_window, pa, pa_mask);   // We need to recreate the picture.

    Pixmap back_buffer_pixmap = XCreatePixmap(display(), rootWindow().window(), rootWindow().width(), rootWindow().height(), 32);
    m_back_buffer_picture->setPixmap(back_buffer_pixmap, true, pa, pa_mask);
}


// Update the background picture.
void XRenderScreen::updateBackgroundPicture() {
    XRenderPictFormat *pict_format;
    if (wmSetRootWindowPixmap()) {
        pict_format = XRenderFindVisualFormat(display(), rootWindow().visual());
    } else {
        pict_format = XRenderFindStandardFormat(display(), PictStandardARGB32);
    }

    if (!pict_format) {
        throw RuntimeException("Cannot find the required picture format.");
    }

    XRenderPictureAttributes pa;
    pa.subwindow_mode = IncludeInferiors;
    long pa_mask = CPSubwindowMode;

    if (!m_root_picture) {
        m_root_picture = new XRenderPicture(*this, pict_format, m_pict_filter);
    } else {
        m_root_picture->setPictFormat(pict_format);
    }
    m_root_picture->setPixmap(rootWindowPixmap(), false, pa, pa_mask);
    m_root_changed = false;
}


//--- SCREEN RENDERING ---------------------------------------------------------

// Renders the screen's contents.
void XRenderScreen::renderScreen() {
    clipBackBufferToDamage();

    renderBackground();

    std::list<BaseCompWindow*>::const_iterator it = allWindows().begin();
    while (it != allWindows().end()) {
        if (!(*it)->isIgnored() && (*it)->isMapped()) {
            renderWindow(*(dynamic_cast<XRenderWindow*>(*it)));
        }
        ++it;
    }

    if ((reconfigureRectangle().width != 0) && (reconfigureRectangle().height != 0)) {
        renderReconfigureRect();
    }
    
    renderExtraJobs();

    swapBuffers();
}

// Clips the backbuffer picture to damaged area.
void XRenderScreen::clipBackBufferToDamage() {
    XRenderPlugin *plugin = NULL;

    m_plugin_damage_rects.clear();
    forEachPlugin(i, plugin) {
        const std::vector<XRectangle> &window_damage = plugin->damagedAreas();
        m_plugin_damage_rects.insert(m_plugin_damage_rects.end(), window_damage.begin(), window_damage.end());
    }
    XFixesSetRegion(display(), m_plugin_damage, (XRectangle*)(m_plugin_damage_rects.data()), m_plugin_damage_rects.size());

    XserverRegion all_damage = damagedScreenArea();
    XFixesUnionRegion(display(), all_damage, all_damage, m_plugin_damage);

    XFixesSetPictureClipRegion(display(), m_back_buffer_picture->pictureHandle(), 0, 0, all_damage);
}

// Perform a rendering job on the back buffer picture.
void XRenderScreen::executeRenderingJob(const XRenderRenderingJob &job) {
    if (job.operation != PictOpClear) {
        Picture source = ((job.source_picture) ? (job.source_picture->pictureHandle()) : (None));
        Picture mask = ((job.mask_picture) ? (job.mask_picture->pictureHandle()) : (None));

        XRenderComposite(display(), job.operation, source, mask,
                         m_back_buffer_picture->pictureHandle(), job.source_x, job.source_y,
                         job.mask_x, job.mask_y, job.destination_x, job.destination_y, job.width, job.height);
    }
}

// Render the desktop wallpaper.
// TODO: Simply make the window transparent.
void XRenderScreen::renderBackground() {
    // React to desktop background change.
    if (m_root_changed) {
        updateBackgroundPicture();
    }

    // Draw the desktop.
    XRenderComposite(display(), PictOpSrc, m_root_picture->pictureHandle(), None, m_back_buffer_picture->pictureHandle(),
                     0, 0, 0, 0, 0, 0, rootWindow().width(), rootWindow().height());

    // Additional rendering actions.
    XRenderPlugin *plugin = NULL;
    XRenderRenderingJob job;
    
    forEachPlugin(i, plugin) {
        std::vector<XRenderRenderingJob> jobs = plugin->postBackgroundRenderingActions();
        for (size_t j = 0; j < jobs.size(); j++) {
            executeRenderingJob(jobs[j]);
        }
    }
}

// Perform extra rendering jobs from plugins.
void XRenderScreen::renderExtraJobs() {
    XRenderPlugin *plugin = NULL;
    XRenderRenderingJob job;

    forEachPlugin(i, plugin) {
        std::vector<XRenderRenderingJob> jobs = plugin->extraRenderingActions();
        for (size_t j = 0; j < jobs.size(); j++) {
            executeRenderingJob(jobs[j]);
        }
        plugin->postExtraRenderingActions();
    }
}

// Render the reconfigure rectangle.
void XRenderScreen::renderReconfigureRect() {
    XRenderPlugin *plugin = NULL;
    XRectangle rect = reconfigureRectangle();

    XSetForeground(display(), m_back_buffer_picture->gcHandle(), XWhitePixel(display(), screenNumber()));
    XSetFunction(display(), m_back_buffer_picture->gcHandle(), GXxor);
    XSetLineAttributes(display(), m_back_buffer_picture->gcHandle(), 1, LineSolid, CapNotLast, JoinMiter);

    forEachPlugin(i, plugin) {
        plugin->recRectRenderingJobInit(rect, m_back_buffer_picture->gcHandle());
    }
    XDrawRectangles(display(), m_back_buffer_picture->drawableHandle(),
                    m_back_buffer_picture->gcHandle(), &rect, 1);
}

// Render a particular window onto the screen.
void XRenderScreen::renderWindow(XRenderWindow &window) {
    XRenderPlugin *plugin = NULL;
    XRenderRenderingJob job;

    // Update window contents.
    if (window.isDamaged()) {
        window.updateContents();
    }

    // This might happen if the window is mapped and unmapped in the same
    // frame, but the compositor hasn't received the unmap event yet.
    if ((window.contentPicture()->pictureHandle() == None)
            || (window.maskPicture()->pictureHandle() == None)) {
        return;
    }

    // Extra rendering actions before window is drawn.
    forEachPlugin(i, plugin) {
        std::vector<XRenderRenderingJob> jobs = plugin->preWindowRenderingActions(window);
        for (size_t j = 0; j < jobs.size(); j++) {
            executeRenderingJob(jobs[j]);
        }
    }

    // Draw the window.
    job.operation = PictOpOver;
    job.source_picture = window.contentPicture();
    job.mask_picture = window.maskPicture();
    job.source_x = 0;
    job.source_y = 0;
    job.mask_x = 0;
    job.mask_y = 0;
    job.destination_x = window.x();
    job.destination_y = window.y();
    job.width = window.realWidth();
    job.height = window.realHeight();

    forEachPlugin(i, plugin) {
        plugin->windowRenderingJobInit(window, job);
    }
    executeRenderingJob(job);

    // Extra rendering actions after window is drawn.
    forEachPlugin(i, plugin) {
        std::vector<XRenderRenderingJob> jobs = plugin->postWindowRenderingActions(window);
        for (size_t j = 0; j < jobs.size(); j++) {
            executeRenderingJob(jobs[j]);
        }
    }
}

// Swap back and front buffers.
void XRenderScreen::swapBuffers() {
    XRenderComposite(display(), PictOpSrc, m_back_buffer_picture->pictureHandle(), None, m_rendering_picture->pictureHandle(),
                     0, 0, 0, 0, 0, 0, rootWindow().width(), rootWindow().height());
}


//--- SPECIALIZED WINDOW MANIPULATION FUNCTIONS --------------------------------

// Creates a window object from its XID.
BaseCompWindow *XRenderScreen::createWindowObject(Window window) {
    XRenderWindow *new_window = new XRenderWindow(*this, window, m_pict_filter);
    return new_window;
}
