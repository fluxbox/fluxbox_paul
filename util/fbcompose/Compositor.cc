/** Compositor.cc file for the fluxbox compositor. */

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


#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif  // HAVE_CONFIG_H

#include "Compositor.hh"

#include "Atoms.hh"
#include "BaseScreen.hh"
#include "CompositorConfig.hh"
#include "Constants.hh"
#include "Logging.hh"
#include "OpenGLScreen.hh"
#include "XRenderScreen.hh"

#ifdef USE_OPENGL_COMPOSITING
    #include <GL/glxew.h>
    #include <GL/glx.h>
#endif  // USE_OPENGL_COMPOSITING

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xfixes.h>
#ifdef XINERAMA
    #include <X11/extensions/Xinerama.h>
#endif  // XINERAMA
#ifdef USE_XRENDER_COMPOSITING
    #include <X11/extensions/Xrender.h>
#endif  // USE_XRENDER_COMPOSITING
#include <X11/Xutil.h>

#include <sstream>

#ifdef HAVE_CTIME
    #include <ctime>
#else
#ifdef HAVE_TIME_H
    #include <time.h>
#endif
#endif

#include <csignal>

using namespace FbCompositor;


//--- CONSTANTS ----------------------------------------------------------------

/** Length of the error buffer in X error handler. */
const int ERROR_BUFFER_LENGTH = 128;

/** Name of the relevant error database with X error messages in X error handler. */
const char ERROR_DB_TEXT_NAME[] = "XRequest";

/** Default name for unknown requests in X error handler. */
const char REQUEST_NAME_UNKNOWN_MESSAGE[] = "<UNKNOWN>";

/** How many microseconds to sleep before restarting the event loop. */
const int SLEEP_TIME = 5000;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// The constructor.
Compositor::Compositor(const CompositorConfig &config) :
    App(config.displayName().c_str()) {

    if (config.synchronize()) {
        XSynchronize(display(), True);
    }

    if (config.renderingMode() == RM_ServerAuto) {
        throw InitException("Compositor class does not provide the serverauto renderer.");
    }
    m_rendering_mode = config.renderingMode();

    if (config.showXErrors()) {
        XSetErrorHandler(&printXError);
    } else {
        XSetErrorHandler(&ignoreXError);
    }

    initAllExtensions();

    int screen_count = XScreenCount(display());
    m_screens.reserve(screen_count);

    for (int i = 0; i < screen_count; i++) {
        Window cm_selection_owner = getCMSelectionOwnership(i);

        switch (m_rendering_mode) {
#ifdef USE_OPENGL_COMPOSITING
        case RM_OpenGL :
            m_screens.push_back(new OpenGLScreen(i, config));
            break;
#endif  // USE_OPENGL_COMPOSITING
#ifdef USE_XRENDER_COMPOSITING
        case RM_XRender :
            m_screens.push_back(new XRenderScreen(i, config));
            break;
#endif  // USE_XRENDER_COMPOSITING
        default :
            throw InitException("Unknown rendering mode selected.");
            break;
        }

        m_screens[i]->ignoreWindow(cm_selection_owner);
    }

    initHeads();
    for (size_t i = 0; i < m_screens.size(); i++) {
        m_screens[i]->initPlugins(config);
        m_screens[i]->initWindows();
    }

    m_timer.setTickSize(1000000 / config.framesPerSecond());
    m_timer.start();

    XFlush(display());

    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);
}

// Destructor.
Compositor::~Compositor() {
    std::vector<BaseScreen*>::iterator it = m_screens.begin();
    while (it != m_screens.end()) {
        delete *it;
        ++it;
    }
}


//--- INITIALIZATION FUNCTIONS -----------------------------------------

// Acquires the ownership of compositing manager selections.
Window Compositor::getCMSelectionOwnership(int screen_number) {
    Atom cm_atom = Atoms::compositingSelectionAtom(screen_number);

    Window cur_owner = XGetSelectionOwner(display(), cm_atom);
    if (cur_owner != None) {
        // TODO: More detailed message - what is the other program?
        throw InitException("Another compositing manager is running.");
    }

    cur_owner = XCreateSimpleWindow(display(), XRootWindow(display(), screen_number), -10, -10, 1, 1, 0, None, None);
    XmbSetWMProperties(display(), cur_owner, APP_NAME, APP_NAME, NULL, 0, NULL, NULL, NULL);
    XSetSelectionOwner(display(), cm_atom, cur_owner, CurrentTime);

    return cur_owner;
}

// Initializes X's extensions.
void Compositor::initAllExtensions() {
#ifdef USE_OPENGL_COMPOSITING
    if (m_rendering_mode == RM_OpenGL) {
        initExtension("GLX", &glXQueryExtension, &glXQueryVersion, 1, 3, m_glx_event_base, m_glx_error_base);
        initExtension("XComposite", &XCompositeQueryExtension, &XCompositeQueryVersion, 0, 4, m_composite_event_base, m_composite_error_base);
        initExtension("XDamage", &XDamageQueryExtension, &XDamageQueryVersion, 1, 0, m_damage_event_base, m_damage_error_base);
        initExtension("XFixes", &XFixesQueryExtension, &XFixesQueryVersion, 2, 0, m_fixes_event_base, m_fixes_error_base);
        initExtension("XShape", &XShapeQueryExtension, &XShapeQueryVersion, 1, 1, m_shape_event_base, m_shape_error_base);
    } else
#endif  // USE_OPENGL_COMPOSITING

#ifdef USE_XRENDER_COMPOSITING
    if (m_rendering_mode == RM_XRender) {
        initExtension("XComposite", &XCompositeQueryExtension, &XCompositeQueryVersion, 0, 4, m_composite_event_base, m_composite_error_base);
        initExtension("XDamage", &XDamageQueryExtension, &XDamageQueryVersion, 1, 0, m_damage_event_base, m_damage_error_base);
        initExtension("XFixes", &XFixesQueryExtension, &XFixesQueryVersion, 2, 0, m_fixes_event_base, m_fixes_error_base);
        initExtension("XRender", &XRenderQueryExtension, &XRenderQueryVersion, 0, 1, m_render_event_base, m_render_error_base);
        initExtension("XShape", &XShapeQueryExtension, &XShapeQueryVersion, 1, 1, m_shape_event_base, m_shape_error_base);
    } else
#endif  // USE_XRENDER_COMPOSITING

    { }
}

// Initializes a particular X server extension.
void Compositor::initExtension(const char *extension_name, QueryExtensionFunction extension_func,
                               QueryVersionFunction version_func, const int min_major_ver, const int min_minor_ver,
                               int &event_base, int &error_base) {
    int major_ver;
    int minor_ver;

    // Check that the extension exists.
    if (!(*extension_func)(display(), &event_base, &error_base)) {
        event_base = -1;
        error_base = -1;

        std::stringstream ss;
        ss << extension_name << " extension not found.";
        throw InitException(ss.str());
    }

    // Get extension version.
    if (!(*version_func)(display(), &major_ver, &minor_ver)) {
        event_base = -1;
        error_base = -1;

        std::stringstream ss;
        ss << "Could not query the version of " << extension_name << " extension.";
        throw InitException(ss.str());
    }

    // Make sure the extension version is at least what we require.
    if ((major_ver < min_major_ver) || ((major_ver == min_major_ver) && (minor_ver < min_minor_ver))) {
        event_base = -1;
        error_base = -1;

        std::stringstream ss;
        ss << "Unsupported " << extension_name << " extension version found (required >=" << min_major_ver
           << "." << min_minor_ver << ", got " << major_ver << "." << minor_ver << ").";
        throw InitException(ss.str());
    }
}

// Initializes monitor heads on every screen.
void Compositor::initHeads() {
    HeadMode head_mode = Heads_One;

#ifdef XINERAMA
    try {
        initExtension("Xinerama", &XineramaQueryExtension, &XCompositeQueryVersion, 0, 0, m_xinerama_event_base, m_xinerama_error_base);
        if (XineramaIsActive(display())) {
            head_mode = Heads_Xinerama;
        }
    } catch (const InitException &e) { }

    if (head_mode != Heads_Xinerama) {
        fbLog_warn << "Could not initialize Xinerama." << std::endl;
    }
#endif  // XINERAMA

    for (size_t i = 0; i < m_screens.size(); i++) {
        m_screens[i]->updateHeads(head_mode);
    }
}


//--- EVENT LOOP ---------------------------------------------------------------

// The event loop.
void Compositor::eventLoop() {
    union {
        XEvent event_x;
        XDamageNotifyEvent event_x_damage_notify;
        XShapeEvent event_x_shape;
    } event_union;
    XEvent &event = event_union.event_x;

    int event_screen;
    timespec sleep_timespec = { 0, SLEEP_TIME * 1000 };

    while (!done()) {
        while (XPending(display())) {
            XNextEvent(display(), &event);

            event_screen = screenOfEvent(event);
            if (event_screen < 0) {
                fbLog_info << "Event " << std::dec << event.xany.serial << " (window " << std::hex << event.xany.window
                           << ", type " << std::dec << event.xany.type << ") does not affect any managed windows, skipping."
                           << std::endl;
                continue;
            }

            switch (event.type) {
            case CirculateNotify :
                m_screens[event_screen]->circulateWindow(event.xcirculate.window, event.xcirculate.place);
                fbLog_debug << "CirculateNotify on " << std::hex << event.xcirculate.window << std::endl;
                break;

            case ConfigureNotify :
                m_screens[event_screen]->reconfigureWindow(event.xconfigure);
                fbLog_debug << "ConfigureNotify on " << std::hex << event.xconfigure.window << std::endl;
                break;

            case CreateNotify :
                m_screens[event_screen]->createWindow(event.xcreatewindow.window);
                fbLog_debug << "CreateNotify on " << std::hex << event.xcreatewindow.window << std::endl;
                break;

            case DestroyNotify :
                m_screens[event_screen]->destroyWindow(event.xdestroywindow.window);
                fbLog_debug << "DestroyNotify on " << std::hex << event.xdestroywindow.window << std::endl;
                break;

            case Expose :
                m_screens[event_screen]->damageWindow(event.xexpose.window, getExposedRect(event.xexpose));
                fbLog_debug << "Expose on " << std::hex << event.xexpose.window << std::endl;
                break;

            case GravityNotify :
                fbLog_debug << "GravityNotify on " << std::hex << event.xgravity.window << std::endl;
                break;

            case MapNotify :
                m_screens[event_screen]->mapWindow(event.xmap.window);
                fbLog_debug << "MapNotify on " << std::hex << event.xmap.window << std::endl;
                break;

            case PropertyNotify :
                m_screens[event_screen]->updateWindowProperty(event.xproperty.window, event.xproperty.atom, event.xproperty.state);
                fbLog_debug << "PropertyNotify on " << std::hex << event.xproperty.window << " ("
                            << XGetAtomName(display(), event.xproperty.atom) << ")" << std::endl;
                break;

            case ReparentNotify :
                m_screens[event_screen]->reparentWindow(event.xreparent.window, event.xreparent.parent);
                fbLog_debug << "ReparentNotify on " << std::hex << event.xreparent.window << " (parent "
                            << event.xreparent.parent << ")" << std::endl;
                break;

            case UnmapNotify :
                m_screens[event_screen]->unmapWindow(event.xunmap.window);
                fbLog_debug << "UnmapNotify on " << std::hex << event.xunmap.window << std::endl;
                break;

            default :
                if (event.type == (m_damage_event_base + XDamageNotify)) {
                    XDamageNotifyEvent damageEvent = event_union.event_x_damage_notify;
                    m_screens[event_screen]->damageWindow(damageEvent.drawable, damageEvent.area);
                    fbLog_debug << "DamageNotify on " << std::hex << damageEvent.drawable << std::endl;

                } else if (event.type == (m_shape_event_base + ShapeNotify)) {
                    XShapeEvent shapeEvent = event_union.event_x_shape;
                    m_screens[event_screen]->updateShape(shapeEvent.window);
                    fbLog_debug << "ShapeNotify on " << std::hex << shapeEvent.window << std::endl;

                } else {
                    fbLog_debug << "Other event " << std::dec << event.xany.type << " received on screen "
                                << event_screen << " and window " << std::hex << event.xany.window << std::endl;
                }
                break;
            }
        }

        if (m_timer.newElapsedTicks()) {
            for (size_t i = 0; i < m_screens.size(); i++) {
                m_screens[i]->renderScreen();
                m_screens[i]->clearScreenDamage();
            }
            XSync(display(), False);

            fbLog_debugDump << m_screens.size() << " screen(s) available." << std::endl;
            for (size_t i = 0; i < m_screens.size(); i++) {
                fbLog_debugDump << *m_screens[i];
            }
            fbLog_debugDump << "======================================" << std::endl;
        } else {
            nanosleep(&sleep_timespec, NULL);
        }
    }
}


//--- INTERNAL FUNCTIONS -----------------------------------------------

// Returns the exposed area in a XExposeEvent as an XRectangle.
XRectangle Compositor::getExposedRect(const XExposeEvent &event) {
    XRectangle rect = { event.x, event.y, event.width, event.height };
    return rect;
}

// Locates the screen an event affects. Returns -1 on failure.
int Compositor::screenOfEvent(const XEvent &event) {
    if (m_screens.size() == 1) {
        return 0;
    } else {
        for (size_t i = 0; i < m_screens.size(); i++) {
            if ((event.xany.window == m_screens[i]->rootWindow().window())
                    || (m_screens[i]->isWindowManaged(event.xany.window))) {
                return i;
            }
        }
    }

    return -1;
}


//--- VARIOUS HANDLERS ---------------------------------------------------------

// Custom signal handler.
void FbCompositor::handleSignal(int sig) {
    if ((sig == SIGINT) || (sig == SIGTERM)) {
        FbTk::App::instance()->end();
    }
}


// Custom X error handler (ignore).
int FbCompositor::ignoreXError(Display * /*display*/, XErrorEvent * /*error*/) {
    return 0;
}

// Custom X error handler (print, continue).
int FbCompositor::printXError(Display *display, XErrorEvent *error) {
    static std::stringstream ss;
    ss.str("");

    char error_text[ERROR_BUFFER_LENGTH];
    XGetErrorText(display, error->error_code, error_text, ERROR_BUFFER_LENGTH);

    ss << int(error->request_code);

    char request_name[ERROR_BUFFER_LENGTH];
    XGetErrorDatabaseText(display, (char*)(ERROR_DB_TEXT_NAME), (char*)(ss.str().c_str()),
                          (char*)(REQUEST_NAME_UNKNOWN_MESSAGE), request_name, ERROR_BUFFER_LENGTH);

    fbLog_warn << "X Error: " << error_text << " in " << request_name << " request, errorCode="
               << std::dec << int(error->error_code) << ", majorOpCode=" << int(error->request_code)
               << ", minorOpCode=" << int(error->minor_code) << ", resourceId=" << std::hex
               << error->resourceid << "." << std::endl;

    return 0;
}
