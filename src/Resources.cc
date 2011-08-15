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

#include "Resources.hh"

#include "FbTk/Container.hh"
#include "FbTk/StringUtil.hh"
#include "FbTk/Resource.hh"
#include "FbTk/Luamm.hh"

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

//--------------------------------------------------
//---- accessors for some enums with Resource ------
//--------------------------------------------------
namespace FbTk {

template<>
const EnumTraits<WinButtonType>::Pair EnumTraits<WinButtonType>::s_map[] = {
    { "Shade",    SHADEBUTTON },
    { "Minimize", MINIMIZEBUTTON },
    { "Maximize", MAXIMIZEBUTTON },
    { "Close",    CLOSEBUTTON },
    { "Stick",    STICKBUTTON },
    { "MenuIcon", MENUICONBUTTON },
    { NULL,       MENUICONBUTTON }
};

template<>
const EnumTraits<TabsAttachArea>::Pair EnumTraits<TabsAttachArea>::s_map[] = {
    { "Titlebar", ATTACH_AREA_TITLEBAR },
    { "Window",   ATTACH_AREA_WINDOW },
    { NULL,       ATTACH_AREA_WINDOW }
};

template<>
const EnumTraits<LayerType>::Pair EnumTraits<LayerType>::s_map[] = {
    { "Menu",     LAYERMENU },
    { "1",        LAYER1 },
    { "AboveDock",LAYERABOVE_DOCK },
    { "3",        LAYER3 },
    { "Dock",     LAYERDOCK },
    { "5",        LAYER5 },
    { "Top",      LAYERTOP },
    { "7",        LAYER7 },
    { "Normal",   LAYERNORMAL },
    { "9",        LAYER9 },
    { "Bottom",   LAYERBOTTOM },
    { "11",       LAYER11 },
    { "Desktop",  LAYERDESKTOP },
    { NULL,       LAYERDESKTOP }
};

template<>
const EnumTraits<Container::Alignment>::Pair EnumTraits<Container::Alignment>::s_map[] = {
    { "Left",     Container::LEFT },
    { "Right",    Container::RIGHT },
    { "Relative", Container::RELATIVE }, 
    { NULL,       Container::RELATIVE }
};

template<>
const EnumTraits<Placement>::Pair EnumTraits<Placement>::s_map[] = {
    { "TopLeft",      TOPLEFT },
    { "TopCenter",    TOPCENTER },
    { "TopRight",     TOPRIGHT },
    { "BottomLeft",   BOTTOMLEFT },
    { "BottomCenter", BOTTOMCENTER },
    { "BottomRight",  BOTTOMRIGHT },
    { "LeftBottom",   LEFTBOTTOM },
    { "LeftCenter",   LEFTCENTER },
    { "LeftTop",      LEFTTOP },
    { "RightBottom",  RIGHTBOTTOM },
    { "RightCenter",  RIGHTCENTER },
    { "RightTop",     RIGHTTOP },
    { "Top",          TOPCENTER },
    { "Bottom",       BOTTOMCENTER },
    { "Left",         LEFTCENTER },
    { "Right",        RIGHTCENTER },
    { NULL,           RIGHTTOP }
};

template <>
const EnumTraits<PlacementPolicy>::Pair EnumTraits<PlacementPolicy>::s_map[] = {
    { "RowSmartPlacement",      ROWSMARTPLACEMENT },
    { "ColSmartPlacement",      COLSMARTPLACEMENT },
    { "RowMinOverlapPlacement", ROWMINOVERLAPPLACEMENT },
    { "ColMinOverlapPlacement", COLMINOVERLAPPLACEMENT },
    { "UnderMousePlacement",    UNDERMOUSEPLACEMENT },
    { "CascadePlacement",       CASCADEPLACEMENT },
    { NULL,                     CASCADEPLACEMENT }
};

template <>
const EnumTraits<RowDirection>::Pair EnumTraits<RowDirection>::s_map[] = {
    { "LeftToRight",            LEFTRIGHTDIRECTION },
    { "RightToLeft",            RIGHTLEFTDIRECTION },
    { NULL,                     RIGHTLEFTDIRECTION },
};

template <>
const EnumTraits<ColumnDirection>::Pair EnumTraits<ColumnDirection>::s_map[] = {
    { "TopToBottom",            TOPBOTTOMDIRECTION },
    { "BottomToTop",            BOTTOMTOPDIRECTION },
    { NULL,                     BOTTOMTOPDIRECTION },
};

template<>
const EnumTraits<FocusModel>::Pair EnumTraits<FocusModel>::s_map[] = {
    { "MouseFocus",       MOUSEFOCUS },
    { "StrictMouseFocus", STRICTMOUSEFOCUS },
    { "ClickFocus",       CLICKFOCUS },
    { NULL,               CLICKFOCUS }
};

template<>
const EnumTraits<TabFocusModel>::Pair EnumTraits<TabFocusModel>::s_map[] = {
    { "SloppyTabFocus",   MOUSETABFOCUS },
    { "ClickToTabFocus",  CLICKTABFOCUS },
    { NULL,               CLICKTABFOCUS }
};

} // end namespace FbTk
