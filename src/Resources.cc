// Resources.cc for Fluxbox Window Manager
// Copyright (c) 2004 - 2006 Henrik Kinnunen (fluxgen at fluxbox dot org)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

// holds main resource functions

#include "FbTk/StringUtil.hh"
#include "FbTk/Resource.hh"
#include "FbTk/Luamm.hh"
#include "WinButton.hh"

#include "fluxbox.hh"

#include "Layer.hh"

#include <stdio.h>
#include <string>
#include <vector>
#ifdef HAVE_CSTRING
  #include <cstring>
#else
  #include <string.h>
#endif

using std::string;
using std::vector;

//-----------------------------------------------------------------
//---- accessors for int, bool, and some enums with Resource ------
//-----------------------------------------------------------------
namespace FbTk {

template<>
const EnumTraits<WinButton::Type>::Pair EnumTraits<WinButton::Type>::s_map[] = {
    { "Shade",    WinButton::SHADE },
    { "Minimize", WinButton::MINIMIZE },
    { "Maximize", WinButton::MAXIMIZE },
    { "Close",    WinButton::CLOSE },
    { "Stick",    WinButton::STICK },
    { "MenuIcon", WinButton::MENUICON },
    { NULL,       WinButton::MENUICON }
};

template<>
const EnumTraits<Fluxbox::TabsAttachArea>::Pair EnumTraits<Fluxbox::TabsAttachArea>::s_map[] = {
    { "Titlebar", Fluxbox::ATTACH_AREA_TITLEBAR },
    { "Window",   Fluxbox::ATTACH_AREA_WINDOW },
    { NULL,       Fluxbox::ATTACH_AREA_WINDOW }
};

template<>
const EnumTraits<ResourceLayer::Type>::Pair EnumTraits<ResourceLayer::Type>::s_map[] = {
    { "Menu",     ResourceLayer::MENU },
    { "1",        ResourceLayer::LAYER1 },
    { "AboveDock",ResourceLayer::ABOVE_DOCK },
    { "3",        ResourceLayer::LAYER3 },
    { "Dock",     ResourceLayer::DOCK },
    { "5",        ResourceLayer::LAYER5 },
    { "Top",      ResourceLayer::TOP },
    { "7",        ResourceLayer::LAYER7 },
    { "Normal",   ResourceLayer::NORMAL },
    { "9",        ResourceLayer::LAYER9 },
    { "Bottom",   ResourceLayer::BOTTOM },
    { "11",       ResourceLayer::LAYER11 },
    { "Desktop",  ResourceLayer::DESKTOP },
    { NULL,       ResourceLayer::DESKTOP }
};

} // end namespace FbTk
