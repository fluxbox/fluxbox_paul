/** BasePlugin.cc file for the fluxbox compositor. */

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


#include "BasePlugin.hh"

#include "BaseScreen.hh"

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
BasePlugin::BasePlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &/*args*/) :
    m_screen(screen) {

    m_display = (Display*)(screen.display());
}

// Destructor.
BasePlugin::~BasePlugin() { }


//--- WINDOW EVENT CALLBACKS ---------------------------------------------------

// Called, whenever a window becomes ignored.
void BasePlugin::windowBecameIgnored(const BaseCompWindow &/*window*/) { }

// Called, whenever a window is circulated.
void BasePlugin::windowCirculated(const BaseCompWindow &/*window*/, int /*place*/) { }

// Called, whenever a new window is created.
void BasePlugin::windowCreated(const BaseCompWindow &/*window*/) { }

// Called, whenever a window is damaged.
void BasePlugin::windowDamaged(const BaseCompWindow &/*window*/) { }

// Called, whenever a window is destroyed.
void BasePlugin::windowDestroyed(const BaseCompWindow &/*window*/) { }

// Called, whenever a window is mapped.
void BasePlugin::windowMapped(const BaseCompWindow &/*window*/) { }

// Called, whenever window's property is changed.
void BasePlugin::windowPropertyChanged(const BaseCompWindow &/*window*/, Atom /*property*/, int /*state*/) { }

// Called, whenever a window is reconfigured.
void BasePlugin::windowReconfigured(const BaseCompWindow &/*window*/) { }

// Called, whenever window's shape changes.
void BasePlugin::windowShapeChanged(const BaseCompWindow &/*window*/) { }

// Called, whenever a window is unmapped.
void BasePlugin::windowUnmapped(const BaseCompWindow &/*window*/) { }


//--- SCREEN CHANGES -----------------------------------------------------------

// Notifies the plugin of a background change.
void BasePlugin::setRootPixmapChanged() { }

// Notifies the plugin of a root window change.
void BasePlugin::setRootWindowSizeChanged() { }
