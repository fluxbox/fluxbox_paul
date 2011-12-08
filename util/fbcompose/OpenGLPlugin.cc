/** OpenGLPlugin.cc file for the fluxbox compositor. */

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


#include "OpenGLPlugin.hh"

#include "Exceptions.hh"
#include "OpenGLScreen.hh"
#include "OpenGLWindow.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Costructor.
OpenGLPlugin::OpenGLPlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args) :
    BasePlugin(screen, args) {
}

// Destructor.
OpenGLPlugin::~OpenGLPlugin() { }


//--- OTHER INITIALIZATION -----------------------------------------------------

// Initialize OpenGL-specific code.
void OpenGLPlugin::initOpenGL(OpenGLShaderProgramPtr /*shader_program*/) { }


//--- ACCESSORS ----------------------------------------------------------------

// Returns a reference screen object, cast into the correct class.
const OpenGLScreen &OpenGLPlugin::openGLScreen() const {
    static const OpenGLScreen &s = dynamic_cast<const OpenGLScreen&>(BasePlugin::screen());
    return s;
}


//--- RENDERING ACTIONS --------------------------------------------------------

// Background rendering initialization.
void OpenGLPlugin::backgroundRenderInit(int /*part_id*/) { }

// Background rendering cleanup.
void OpenGLPlugin::backgroundRenderCleanup(int /*part_id*/) { }

// Post background rendering actions.
const std::vector<OpenGLRenderingJob> &OpenGLPlugin::postBackgroundRenderActions() {
    static std::vector<OpenGLRenderingJob> jobs;
    return jobs;
}


// Pre window rendering actions and jobs.
const std::vector<OpenGLRenderingJob> &OpenGLPlugin::preWindowRenderActions(const OpenGLWindow &/*window*/) {
    static std::vector<OpenGLRenderingJob> jobs;
    return jobs;
}

// Window rendering initialization.
void OpenGLPlugin::windowRenderInit(const OpenGLWindow &/*window*/, int /*part_id*/) { }

// Window rendering cleanup.
void OpenGLPlugin::windowRenderCleanup(const OpenGLWindow &/*window*/, int /*part_id*/) { }

// Post window rendering actions and jobs.
const std::vector<OpenGLRenderingJob> &OpenGLPlugin::postWindowRenderActions(const OpenGLWindow &/*window*/) {
    static std::vector<OpenGLRenderingJob> jobs;
    return jobs;
}


// Reconfigure rectangle rendering initialization.
void OpenGLPlugin::recRectRenderInit(const XRectangle &/*rec_rect*/) { }

// Reconfigure rectangle rendering cleanup.
void OpenGLPlugin::recRectRenderCleanup(const XRectangle &/*rec_rect*/) { }


// Extra rendering actions and jobs.
const std::vector<OpenGLRenderingJob> &OpenGLPlugin::extraRenderingActions() {
    static std::vector<OpenGLRenderingJob> jobs;
    return jobs;
}

// Post extra rendering actions.
void OpenGLPlugin::postExtraRenderingActions() { }


// Null rendering job initialization.
void OpenGLPlugin::nullRenderInit() { }
