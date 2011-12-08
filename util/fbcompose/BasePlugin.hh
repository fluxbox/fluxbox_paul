/** BasePlugin.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_BASEPLUGIN_HH
#define FBCOMPOSITOR_BASEPLUGIN_HH

#include "Enumerations.hh"

#include "FbTk/FbString.hh"

#include <X11/Xlib.h>

#include <vector>


namespace FbCompositor {

    class BaseCompWindow;
    class BaseScreen;


    /**
     * Base class for compositor plugins.
     */
    class BasePlugin {
    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        BasePlugin(const BaseScreen &screen, const std::vector<FbTk::FbString> &args);

        /** Destructor. */
        virtual ~BasePlugin();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the current connection to the X server. */
        Display *display() const;

        /** \returns the name of the plugin. */
        virtual const char *pluginName() const = 0;

        /** \returns the screen this plugin operates on. */
        const BaseScreen &screen() const;


        //--- WINDOW EVENT CALLBACKS -------------------------------------------

        /** Called, whenever a window becomes ignored. */
        virtual void windowBecameIgnored(const BaseCompWindow &window);

        /** Called, whenever a window is circulated. */
        virtual void windowCirculated(const BaseCompWindow &window, int place);

        /** Called, whenever a new window is created. */
        virtual void windowCreated(const BaseCompWindow &window);

        /** Called, whenever a window is damaged. */
        virtual void windowDamaged(const BaseCompWindow &window);

        /** Called, whenever a window is destroyed. */
        virtual void windowDestroyed(const BaseCompWindow &window);

        /** Called, whenever a window is mapped. */
        virtual void windowMapped(const BaseCompWindow &window);

        /** Called, whenever window's property is changed. */
        virtual void windowPropertyChanged(const BaseCompWindow &window, Atom property, int state);

        /** Called, whenever a window is reconfigured. */
        virtual void windowReconfigured(const BaseCompWindow &window);

        /** Called, whenever window's shape changes. */
        virtual void windowShapeChanged(const BaseCompWindow &window);

        /** Called, whenever a window is unmapped. */
        virtual void windowUnmapped(const BaseCompWindow &window);


        //--- SCREEN CHANGES ---------------------------------------------------

        /** Notifies the screen of a background change. */
        virtual void setRootPixmapChanged();

        /** Notifies the screen of a root window change. */
        virtual void setRootWindowSizeChanged();


    private :
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Copy constructor. */
        BasePlugin(const BasePlugin&);

        /** Assignment operator. */
        BasePlugin &operator=(const BasePlugin&);


        //--- INTERNAL VARIABLES -----------------------------------------------

        /** The current connection to the X server. */
        Display *m_display;

        /** The screen this plugin operates on. */
        const BaseScreen &m_screen;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    /** \returns the current connection to the X server. */
    inline Display *BasePlugin::display() const {
        return m_display;
    }

    // Returns the screen this plugin operates on.
    inline const BaseScreen &BasePlugin::screen() const {
        return m_screen;
    }
}


#endif  // FBCOMPOSITOR_BASEPLUGIN_HH
