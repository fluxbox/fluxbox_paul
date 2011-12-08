/** ServerAutoApp.cc file for the fluxbox compositor. */

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

#include "ServerAutoApp.hh"

#include "Atoms.hh"
#include "CompositorConfig.hh"
#include "Constants.hh"

#include <X11/extensions/Xcomposite.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <sstream>

#ifdef HAVE_CTIME
    #include <ctime>
#else
#ifdef HAVE_TIME_H
    #include <time.h>
#endif  // HAVE_TIME_H
#endif  // HAVE_CTIME

#include <csignal>

using namespace FbCompositor;


//--- CONSTANTS ----------------------------------------------------------------

// How many microseconds to sleep before restarting the event loop.
const int SLEEP_TIME = 10000;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
ServerAutoApp::ServerAutoApp(const CompositorConfig &config) :
    App(config.displayName().c_str()) {

    if (config.renderingMode() != RM_ServerAuto) {
        throw InitException("ServerAutoApp provides only the \"serverauto\" renderer.");
    }
    initComposite();
    initScreens();

    XFlush(display());

    signal(SIGINT, handleSignal_ServerAuto);
    signal(SIGTERM, handleSignal_ServerAuto);
}

// Destructor.
ServerAutoApp::~ServerAutoApp() { }


//--- INITIALIZATION FUNCTIONS -------------------------------------------------

// Initialize Composite extension.
void ServerAutoApp::initComposite() {
    int event_base;
    int error_base;
    int major_ver;
    int minor_ver;

    if (!XCompositeQueryExtension(display(), &event_base, &error_base)) {
        throw InitException("Composite extension not found.");
    }
    if (!XCompositeQueryVersion(display(), &major_ver, &minor_ver)) {
        throw InitException("Could not query the version of the Composite extension.");
    }
    if ((major_ver < 0) || ((major_ver == 0) && (minor_ver < 1))) {
        std::stringstream ss;
        ss << "Unsupported Composite extension version found (required >=0.1, got "
           << major_ver << "." << minor_ver << ").";
        throw InitException(ss.str());
    }
}

// Prepare screens.
void ServerAutoApp::initScreens() {
    int screen_count = XScreenCount(display());

    for (int i = 0; i < screen_count; i++) {
        XCompositeRedirectSubwindows(display(), XRootWindow(display(), i), CompositeRedirectAutomatic);

        Atom cm_atom = Atoms::compositingSelectionAtom(i);
        Window cur_owner = XGetSelectionOwner(display(), cm_atom);
        if (cur_owner != None) {
            // TODO: More detailed message - what is the other program?
            throw InitException("Another compositing manager is running.");
        }

        cur_owner = XCreateSimpleWindow(display(), XRootWindow(display(), i), -10, -10, 1, 1, 0, None, None);
        XmbSetWMProperties(display(), cur_owner, APP_NAME, APP_NAME, NULL, 0, NULL, NULL, NULL);
        XSetSelectionOwner(display(), cm_atom, cur_owner, CurrentTime);
    }
}


//--- EVENT LOOP ---------------------------------------------------------------

// Enters the event loop.
void ServerAutoApp::eventLoop() {
    timespec sleep_timespec = { 0, SLEEP_TIME * 1000 };
    while (!done()) {
        nanosleep(&sleep_timespec, NULL);
    }
}


//--- VARIOUS HANDLERS ---------------------------------------------------------

// Custom signal handler for ServerAuto mode.
void FbCompositor::handleSignal_ServerAuto(int sig) {
    if ((sig == SIGINT) || (sig == SIGTERM)) {
        FbTk::App::instance()->end();
    }
}
