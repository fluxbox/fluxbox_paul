/** BaseScreen.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_SCREEN_HH
#define FBCOMPOSITOR_SCREEN_HH

#include "BaseCompWindow.hh"
#include "Enumerations.hh"
#include "PluginManager.hh"

#include <X11/extensions/Xfixes.h>
#include <X11/Xlib.h>

#include <algorithm>
#include <iosfwd>
#include <list>
#include <vector>


namespace FbCompositor {

    class BaseScreen;
    class CompositorConfig;


    //--- SUPPORTING FUNCTIONS -------------------------------------------------

    /** << output stream operator for the BaseScreen class. */
    std::ostream &operator<<(std::ostream& out, const BaseScreen& s);


    //--- BASE CLASS FOR SCREENS -----------------------------------------------

    /**
     * Base class for screen managing classes.
     */
    class BaseScreen {
        //--- FRIEND OPERATORS -------------------------------------------------

        friend std::ostream &operator<<(std::ostream& out, const BaseScreen& s);

    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        BaseScreen(int screen_number, PluginType plugin_type, const CompositorConfig &config);

        /** Destructor. */
        virtual ~BaseScreen();


        //--- OTHER INITIALIZATION ---------------------------------------------

        /** Initializes the screen's plugins. */
        virtual void initPlugins(const CompositorConfig &config);

        /** Initializes all of the windows on the screen. */
        virtual void initWindows();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the current connection to the X server. */
        Display *display();

        /** \returns the current connection to the X server (const version). */
        const Display *display() const;

        /** \returns the vector with the output heads on this screen. */
        const std::vector<XRectangle> &heads() const;

        /** \returns screen's root window. */
        BaseCompWindow &rootWindow();

        /** \returns screen's root window (const version). */
        const BaseCompWindow &rootWindow() const;


        /** \returns the active window XID. */
        Window activeWindow() const;

        /** \returns the XID of the current iconbar item. */
        Window currentIconbarItem() const;

        /** \returns the index of the current workspace. */
        int currentWorkspace() const;

        /** \returns screen's number. */
        int screenNumber() const;

        /** \returns the number of workspaces on this screen. */
        int workspaceCount() const;


        //--- WINDOW MANIPULATION ----------------------------------------------

        /** Circulates a window on this screen. */
        void circulateWindow(Window window, int place);

        /** Creates a new window on this screen. */
        void createWindow(Window window);

        /** Damages a window on this screen. */
        void damageWindow(Window window, const XRectangle &area);

        /** Destroys a window on this screen. */
        void destroyWindow(Window window);

        /** Maps a window on this screen. */
        void mapWindow(Window window);

        /** Updates window's configuration. */
        void reconfigureWindow(const XConfigureEvent &event);

        /** Reparents a window. */
        void reparentWindow(Window window, Window parent);

        /** Updates window's shape. */
        void updateShape(Window window);

        /** Unmaps a window on this screen. */
        void unmapWindow(Window window);

        /** Updates the value of some window's property. */
        void updateWindowProperty(Window window, Atom property, int state);


        /** Marks a particular window as ignored. */
        void ignoreWindow(Window window);

        /** Checks whether a given window is managed by the current screen. */
        bool isWindowManaged(Window window);


        //--- SCREEN MANIPULATION ----------------------------------------------

        /** Removes all accumulated damage from the screen. */
        void clearScreenDamage();

        /** Updates heads on the current screen. */
        void updateHeads(HeadMode head_mode);


        /** Notifies the screen of a background change. */
        virtual void setRootPixmapChanged();

        /** Notifies the screen of a root window change. */
        virtual void setRootWindowSizeChanged();


        //--- SCREEN RENDERING -------------------------------------------------

        /** Renders the screen's contents. */
        virtual void renderScreen() = 0;


    protected:
        //--- PROTECTED ACCESSORS ----------------------------------------------

        /** \returns the list of windows on the screen. */
        const std::list<BaseCompWindow*> &allWindows() const;

        /** \returns the damaged screen area. */
        XserverRegion damagedScreenArea();

        /** \returns the plugin manager. */
        const PluginManager &pluginManager() const;
        
        /** \returns the reconfigure rectangle. */
        XRectangle reconfigureRectangle() const;


        /** \returns the root window pixmap. */
        Pixmap rootWindowPixmap() const;

        /** \returns whether the root window pixmap was set by the WM. */
        bool wmSetRootWindowPixmap() const;


        //--- SPECIALIZED WINDOW MANIPULATION FUNCTIONS ------------------------

        /** Creates a window object from its XID. */
        virtual BaseCompWindow *createWindowObject(Window window) = 0;


    private:
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Copy constructor. */
        BaseScreen(const BaseScreen&);


        //--- PROPERTY UPDATE FUNCTIONS ----------------------------------------

        /** Update stored active window. */
        void updateActiveWindow();

        /** Update the current iconbar item. */
        void updateCurrentIconbarItem();

        /** Update the current workspace index. */
        void updateCurrentWorkspace();

        /** Update stored reconfigure rectangle. */
        void updateReconfigureRect();

        /** Update stored root window pixmap. */
        void updateRootWindowPixmap(Pixmap new_pixmap = None);

        /** Update the number of workspaces. */
        void updateWorkspaceCount();


        //--- SCREEN DAMAGE FUNCTIONS ------------------------------------------

        /** Damages the reconfigure rectangle on the screen. */
        void damageReconfigureRect();

        /** Damages the given rectangle on the screen. */
        void damageScreenArea(XRectangle area);

        /** Damages the area in the given window. */
        void damageWindowArea(BaseCompWindow *window, XRectangle area);

        /** Damages the area taken by the given window. */
        void damageWholeWindowArea(BaseCompWindow *window);


        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** \returns the first managed ancestor of a window. */
        std::list<BaseCompWindow*>::iterator getFirstManagedAncestorIterator(Window window);

        /** \returns the parent of a given window. */
        Window getParentWindow(Window window);

        /** \returns an iterator of m_windows that points to the given window. */
        std::list<BaseCompWindow*>::iterator getWindowIterator(Window window);
        
        /** \returns whether the given window is in the ignore list. */
        bool isWindowIgnored(Window window);

        /** Puts a window to a new location on the stack. */
        void restackWindow(std::list<BaseCompWindow*>::iterator &windowIt, Window above);


        //--- MAIN SCREEN DATA -------------------------------------------------

        /** Current connection to the X server. */
        Display *m_display;

        /** Heads of the current display. */
        std::vector<XRectangle> m_heads;

        /** Windows that should be ignored. */
        std::vector<Window> m_ignore_list;

        /** Plugin manager for this screen. */
        PluginManager m_plugin_manager;

        /** Screen's number. */
        int m_screen_number;

        /** Screen's root window. */
        BaseCompWindow m_root_window;

        /** Screen's windows. */
        std::list<BaseCompWindow*> m_windows;


        /** XID of the active window. */
        Window m_active_window_xid;

        /** XID of the current iconbar item. */
        Window m_current_iconbar_item;

        /** The index of the current workspace. */
        int m_current_workspace;

        /** The current reconfigure rectangle. */
        XRectangle m_reconfigure_rect;

        /** The total number of workspaces. */
        int m_workspace_count;


        /** A list of damaged rectangles on the screen. */
        std::vector<XRectangle> m_damaged_screen_rects;

        /** Damaged screen region. */
        XserverRegion m_screen_damage;


        /** Pixmap, containing the desktop background. */
        Pixmap m_root_window_pixmap;

        /** Whether the background pixmap is set by the window manager or this class. */
        bool m_wm_set_root_window_pixmap;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the active window XID.
    inline Window BaseScreen::activeWindow() const {
        return m_active_window_xid;
    }

    // Returns all of screen's windows.
    inline const std::list<BaseCompWindow*> &BaseScreen::allWindows() const {
        return m_windows;
    }

    // Returns the XID of the current iconbar item.
    inline Window BaseScreen::currentIconbarItem() const {
        return m_current_iconbar_item;
    }

    // Returns the index of the current workspace.
    inline int BaseScreen::currentWorkspace() const {
        return m_current_workspace;
    }

    // Returns the current connection to the X server.
    inline Display *BaseScreen::display() {
        return m_display;
    }

    // Returns the current connection to the X server (const version).
    inline const Display *BaseScreen::display() const {
        return m_display;
    }

    // Returns the vector with the output heads on this screen.
    inline const std::vector<XRectangle> &BaseScreen::heads() const {
        return m_heads;
    }

    // Returns the plugin manager.
    inline const PluginManager &BaseScreen::pluginManager() const {
        return m_plugin_manager;
    }

    // Returns the reconfigure rectangle.
    inline XRectangle BaseScreen::reconfigureRectangle() const {
        return m_reconfigure_rect;
    }

    // Returns screen's root window.
    inline BaseCompWindow &BaseScreen::rootWindow() {
        return m_root_window;
    }

    // Returns screen's root window (const version).
    inline const BaseCompWindow &BaseScreen::rootWindow() const {
        return m_root_window;
    }

    // Returns the root window pixmap.
    inline Pixmap BaseScreen::rootWindowPixmap() const {
        return m_root_window_pixmap;
    }

    // Returns the screen's number.
    inline int BaseScreen::screenNumber() const {
        return m_screen_number;
    }

    // Returns whether the root window pixmap was set by the WM.
    inline bool BaseScreen::wmSetRootWindowPixmap() const {
        return m_wm_set_root_window_pixmap;
    }

    // Returns the number of workspaces on this screen.
    inline int BaseScreen::workspaceCount() const {
        return m_workspace_count;
    }
}


#endif  // FBCOMPOSITOR_SCREEN_HH
