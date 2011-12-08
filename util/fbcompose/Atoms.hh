/** Atoms.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_ATOMS_HH
#define FBCOMPOSITOR_ATOMS_HH

#include "Enumerations.hh"

#include <X11/Xlib.h>

#include <algorithm>
#include <vector>


namespace FbCompositor {

    /**
     * The main X atom manager.
     */
    class Atoms {
    public :
        //--- MAIN METHODS -----------------------------------------------------

        /** \returns the _NET_ACTIVE_WINDOW atom. */
        static Atom activeWindowAtom();

        /** \returns the _NET_WM_CM_Sxx atoms. */
        static Atom compositingSelectionAtom(int screen_number);

        /** \returns the _FLUXBOX_CURRENT_ICONBAR_ITEM atom. */
        static Atom currentIconbarItemAtom();

        /** \returns the _NET_WM_WINDOW_OPACITY atom. */
        static Atom opacityAtom();

        /** \returns the _FLUXBOX_RECONFIGURE_RECT atom. */
        static Atom reconfigureRectAtom();

        /** \returns the atoms that (might) correspond to background pixmap (i.e. wallpapers). */
        static std::vector<Atom> rootPixmapAtoms();

        /** \returns the _NET_WM_WINDOW_TYPE atom. */
        static Atom windowTypeAtom();

        /** \returns a vector with atoms and the correspoding WindowType enum members. */
        static std::vector< std::pair<Atom, WindowType> > windowTypeAtomList();

        /** \returns the _WIN_WORKSPACE atom. */
        static Atom workspaceAtom();

        /** \returns the _WIN_WORKSPACE_COUNT atom. */
        static Atom workspaceCountAtom();


    private :
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Default constructor. */
        Atoms();

        /** Copy constructor. */
        Atoms(const Atoms&);

        /** Assignment operator. */
        Atoms &operator=(const Atoms&);
    };
}


#endif  // FBCOMPOSITOR_ATOMS_HH
