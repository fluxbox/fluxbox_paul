/** Enumerations.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_ENUMERATIONS_HH
#define FBCOMPOSITOR_ENUMERATIONS_HH

#include <ostream>


namespace FbCompositor {

    //--- ENUMERATIONS ---------------------------------------------------------

    /** Monitor heads mode enumeration. */
    enum HeadMode { Heads_One, Heads_Xinerama };

    /** Plugin types. */
    enum PluginType { Plugin_OpenGL, Plugin_XRender };

    /** Rendering mode enumeration. */
    enum RenderingMode { RM_OpenGL, RM_XRender, RM_ServerAuto };

    /** Window type enumeration. */
    enum WindowType { WinType_Desktop, WinType_Dialog, WinType_Dock, WinType_Menu,
                      WinType_Normal, WinType_Splash, WinType_Toolbar, WinType_Utility };


    //--- OSTREAM OUTPUT OPERATORS ---------------------------------------------

    // << operator for WindowType.
    std::ostream &operator<<(std::ostream &os, WindowType win_type);
    inline std::ostream &operator<<(std::ostream &os, WindowType win_type) {
        if (win_type == WinType_Desktop) {
            os << "Desktop";
        } else if (win_type == WinType_Dialog) {
            os << "Dialog";
        } else if (win_type == WinType_Dock) {
            os << "Dock";
        } else if (win_type == WinType_Menu) {
            os << "Menu";
        } else if (win_type == WinType_Normal) {
            os << "Normal";
        } else if (win_type == WinType_Splash) {
            os << "Splash";
        } else if (win_type == WinType_Toolbar) {
            os << "Toolbar";
        } else if (win_type == WinType_Utility) {
            os << "Utility";
        } else {
            os << "Unknown";
        }
        return os;
    }

}

#endif  // FBCOMPOSITOR_ENUMERATIONS_HH
