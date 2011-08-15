// Resources.hh for Fluxbox Window Manager
// Copyright (c) 2011 Pavel Labath (pavelo at centrum dot sk)
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

#ifndef RESOURCES_HH
#define RESOURCES_HH

#include "FbTk/Resource.hh"

enum Placement {
    // top and bottom placement
    TOPLEFT = 1, TOPCENTER, TOPRIGHT,
    BOTTOMLEFT, BOTTOMCENTER, BOTTOMRIGHT,
    // left and right placement
    LEFTBOTTOM, LEFTCENTER, LEFTTOP,
    RIGHTBOTTOM, RIGHTCENTER, RIGHTTOP,
};

enum PlacementPolicy { 
    ROWSMARTPLACEMENT, 
    COLSMARTPLACEMENT,
    COLMINOVERLAPPLACEMENT,
    ROWMINOVERLAPPLACEMENT,
    CASCADEPLACEMENT,
    UNDERMOUSEPLACEMENT
};

enum RowDirection { 
    LEFTRIGHTDIRECTION, ///< from left to right
    RIGHTLEFTDIRECTION  ///< from right to left
};

enum ColumnDirection { 
    TOPBOTTOMDIRECTION,  ///< from top to bottom
    BOTTOMTOPDIRECTION   ///< from bottom to top
};

/// main focus model
enum FocusModel { 
    MOUSEFOCUS = 0,  ///< focus follows mouse, but only when the mouse is moving
    CLICKFOCUS,      ///< focus on click
    STRICTMOUSEFOCUS ///< focus always follows mouse, even when stationary
};

/// focus model for tabs
enum TabFocusModel { 
    MOUSETABFOCUS = 0, ///< tab focus follows mouse
    CLICKTABFOCUS  ///< tab focus on click
};

/// draw type for the WinButtons
enum WinButtonType {
    MAXIMIZEBUTTON,
    MINIMIZEBUTTON,
    SHADEBUTTON,
    STICKBUTTON,
    CLOSEBUTTON,
    MENUICONBUTTON
};

/// obsolete
enum TabsAttachArea{ATTACH_AREA_WINDOW= 0, ATTACH_AREA_TITLEBAR};

enum LayerType {
    LAYERMENU = 0,
    LAYER1 = 1,
    LAYERABOVE_DOCK = 2,
    LAYER3 = 3,
    LAYERDOCK = 4,
    LAYER5 = 5,
    LAYERTOP = 6,
    LAYER7 = 7,
    LAYERNORMAL = 8,
    LAYER9 = 9,
    LAYERBOTTOM = 10,
    LAYER11 = 11,
    LAYERDESKTOP = 12,
    NUM_LAYERS = 13
};

typedef FbTk::Resource<Placement, FbTk::EnumTraits<Placement> > PlacementResource;

#endif /* RESOURCES_HH */
