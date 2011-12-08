/** BaseScreen.cc file for the fluxbox compositor. */

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

#include "BaseScreen.hh"

#include "Atoms.hh"
#include "BasePlugin.hh"
#include "CompositorConfig.hh"
#include "Logging.hh"
#include "Utility.hh"

#include "FbTk/App.hh"

#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#ifdef XINERAMA
    #include <X11/extensions/Xinerama.h>
#endif  // XINERAMA

#include <algorithm>
#include <sstream>
#include <ostream>

using namespace FbCompositor;


//--- MACROS -------------------------------------------------------------------

// Macro for plugin iteration.
#define forEachPlugin(i, plugin)                                               \
    (plugin) = ((pluginManager().plugins().size() > 0)                         \
                   ? ((pluginManager().plugins()[0]))                          \
                   : NULL);                                                    \
    for(size_t (i) = 0;                                                        \
        ((i) < pluginManager().plugins().size());                              \
        (i)++,                                                                 \
        (plugin) = (((i) < pluginManager().plugins().size())                   \
                       ? (pluginManager().plugins()[(i)])                      \
                       : NULL))


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
BaseScreen::BaseScreen(int screen_number, PluginType plugin_type, const CompositorConfig &config) :
    m_display(FbTk::App::instance()->display()),
    m_plugin_manager(plugin_type, *this, config.userPluginDir()),
    m_screen_number(screen_number),
    m_root_window(*this, XRootWindow(m_display, m_screen_number), false) {

    m_screen_damage = XFixesCreateRegion(display(), NULL, 0);

    m_active_window_xid = None;
    m_current_iconbar_item = None;
    updateCurrentWorkspace();
    updateReconfigureRect();
    updateWorkspaceCount();

    m_root_window_pixmap = None;
    m_wm_set_root_window_pixmap = true;
    updateRootWindowPixmap();

    long event_mask = PropertyChangeMask | StructureNotifyMask | SubstructureNotifyMask;
    m_root_window.setEventMask(event_mask);

    XCompositeRedirectSubwindows(m_display, m_root_window.window(), CompositeRedirectManual);

    updateHeads(Heads_One);
}

// Destructor.
BaseScreen::~BaseScreen() {
    if (m_screen_damage) {
        XFixesDestroyRegion(display(), m_screen_damage);
    }

    std::list<BaseCompWindow*>::iterator it = m_windows.begin();
    while (it != m_windows.end()) {
        delete *it;
        ++it;
    }
}


//--- OTHER INITIALIZATION -----------------------------------------------------

// Initializes the screen's plugins.
void BaseScreen::initPlugins(const CompositorConfig &config) {
    for(int i = 0; i < config.pluginCount(); i++) {
        m_plugin_manager.createPluginObject(config.pluginName(i), config.pluginArgs(i));
    }
}

// Initializes all of the windows on the screen.
void BaseScreen::initWindows() {
    Window root;
    Window parent;
    Window *children = 0;
    unsigned int child_count;

    XQueryTree(display(), rootWindow().window(), &root, &parent, &children, &child_count);
    for (unsigned int i = 0; i < child_count; i++) {
        createWindow(children[i]);
    }

    if (children) {
        XFree(children);
    }

    updateActiveWindow();
    updateCurrentIconbarItem();
}


//--- WINDOW MANIPULATION ------------------------------------------------------

// Circulates a window on this screen.
void BaseScreen::circulateWindow(Window window, int place) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        BaseCompWindow *cur_window = *it;
        m_windows.erase(it);

        if (place == PlaceOnTop) {
            m_windows.push_back(cur_window);
        } else {
            m_windows.push_front(cur_window);
        }

        if (!cur_window->isIgnored()) {
            damageWholeWindowArea(*it);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowCirculated(*cur_window, place);
            }
        }
    } else {
        if (window != m_root_window.window()) {
            fbLog_info << "Attempted to circulate an untracked window (" << std::hex << window << ")" << std::endl;
        }
    }
}

// Creates a new window and inserts it into the list of windows.
void BaseScreen::createWindow(Window window) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it == m_windows.end()) {
        BaseCompWindow *new_window = NULL;

        try {
            // TODO: Fix errors coming from window creation (occurs when a
            // window is already destroyed??)
            new_window = createWindowObject(window);
        } catch(const InitException &e) {
            std::stringstream ss;
            ss << "Could not create window " << std::hex << window << " (" << e.what() << ")";
            throw WindowException(ss.str());
        }

        new_window->setEventMask(PropertyChangeMask);
        m_windows.push_back(new_window);

        if (new_window->depth() == 0) {      // If the window is already destroyed, do not render it.
            new_window->setIgnored(true);
        }
        if (isWindowIgnored(window)) {
            new_window->setIgnored(true);
        } 
        
        if (!new_window->isIgnored()) {
            damageWholeWindowArea(new_window);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowCreated(*new_window);
            }
        }
    } else {
        fbLog_info << "Attempted to create a window twice (" << std::hex << window << ")" << std::endl;
    }
}

// Damages a window on this screen.
void BaseScreen::damageWindow(Window window, const XRectangle &area) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->addDamage();

        if (!(*it)->isIgnored()) {
            damageWindowArea((*it), area);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowDamaged(**it);
            }
        }
    } else {
        if (window != m_root_window.window()) {
            fbLog_info << "Attempted to damage an untracked window (" << std::hex << window << ")" << std::endl;
        }
    }
}

// Destroys a window on this screen.
void BaseScreen::destroyWindow(Window window) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        if (!(*it)->isIgnored()) {
            damageWholeWindowArea(*it);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowDestroyed(**it);
            }
        }

        delete *it;
        m_windows.erase(it);
    } else {
        fbLog_info << "Attempted to destroy an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Maps a window on this screen.
void BaseScreen::mapWindow(Window window) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->setMapped();

        if (!(*it)->isIgnored()) {
            damageWholeWindowArea(*it);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowMapped(**it);
            }
        }
    } else {
        fbLog_info << "Attempted to map an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Updates window's configuration.
void BaseScreen::reconfigureWindow(const XConfigureEvent &event) {
    if (event.window == m_root_window.window()) {
        m_root_window.updateGeometry();
        setRootWindowSizeChanged();

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowReconfigured(m_root_window);
        }
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(event.window);
    if (it != m_windows.end()) {
        if (!(*it)->isIgnored()) {
            damageWholeWindowArea(*it);
        }

        (*it)->updateGeometry();
        restackWindow(it, event.above);

        if (!(*it)->isIgnored()) {
            damageWholeWindowArea(*it);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowReconfigured(**it);
            }
        }
    } else {
        fbLog_info << "Attempted to reconfigure an untracked window (" << std::hex << event.window << ")" << std::endl;
    }
}

// Reparents a window.
void BaseScreen::reparentWindow(Window window, Window parent) {
    if (parent == rootWindow().window()) {
        createWindow(window);
    } else {
        destroyWindow(window);
    }
}

// Updates window's shape.
void BaseScreen::updateShape(Window window) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->setClipShapeChanged();

        if (!(*it)->isIgnored()) {
            damageWholeWindowArea(*it);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowShapeChanged(**it);
            }
        }
    } else {
        fbLog_info << "Attempted to update the shape of an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Unmaps a window on this screen.
void BaseScreen::unmapWindow(Window window) {
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->setUnmapped();

        if (!(*it)->isIgnored()) {
            damageWholeWindowArea(*it);

            BasePlugin *plugin = NULL;
            forEachPlugin(i, plugin) {
                plugin->windowUnmapped(**it);
            }
        }
    } else {
        fbLog_info << "Attempted to unmap an untracked window (" << std::hex << window << ")" << std::endl;
    }
}

// Updates the value of some window's property.
void BaseScreen::updateWindowProperty(Window window, Atom property, int state) {
    if ((window == m_root_window.window()) && (property != None) && (state == PropertyNewValue)) {
        if (property == Atoms::activeWindowAtom()) {
            updateActiveWindow();
        } else if (property == Atoms::currentIconbarItemAtom()) {
            updateCurrentIconbarItem();
        } else if (property == Atoms::reconfigureRectAtom()) {
            damageReconfigureRect();    // Damage, so that previous rectangle can be removed.
            updateReconfigureRect();
            damageReconfigureRect();    // Damage, so that new rectangle can be drawn.
        } else if (property == Atoms::workspaceAtom()) {
            updateCurrentWorkspace();
        } else if (property == Atoms::workspaceCountAtom()) {
            updateWorkspaceCount();
        }

        std::vector<Atom> root_pixmap_atoms = Atoms::rootPixmapAtoms();
        for (size_t i = 0; i < root_pixmap_atoms.size(); i++) {
            if (property == root_pixmap_atoms[i]) {
                Pixmap newRootPixmap = m_root_window.singlePropertyValue<Pixmap>(root_pixmap_atoms[i], None);
                updateRootWindowPixmap(newRootPixmap);
                setRootPixmapChanged();     // We don't want this called in the constructor, keep it here.
            }
        }

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowPropertyChanged(m_root_window, property, state);
        }

    } else {
        std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
        if (it != m_windows.end()) {
            (*it)->updateProperty(property, state);

            if (!(*it)->isIgnored()) {
                if (property == Atoms::opacityAtom()) {
                    damageWholeWindowArea(*it);
                }

                BasePlugin *plugin = NULL;
                forEachPlugin(i, plugin) {
                    plugin->windowPropertyChanged(**it, property, state);
                }
            }
        } else {
            if (window != rootWindow().window()) {
                fbLog_info << "Attempted to set the property of an untracked window (" << std::hex << window << ")" << std::endl;
            }
        }
    }
}


// Marks a particular window as ignored.
void BaseScreen::ignoreWindow(Window window) {
    if (isWindowIgnored(window)) {
        return;
    }

    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);
    if (it != m_windows.end()) {
        (*it)->setIgnored(true);

        BasePlugin *plugin = NULL;
        forEachPlugin(i, plugin) {
            plugin->windowBecameIgnored(**it);
        }
    }

    m_ignore_list.push_back(window);
}

// Checks whether a given window is managed by the current screen.
bool BaseScreen::isWindowManaged(Window window) {
    return (getWindowIterator(window) != m_windows.end());
}


//--- SCREEN MANIPULATION ------------------------------------------------------

// Removes all accumulated damage from the screen.
void BaseScreen::clearScreenDamage() {
    m_damaged_screen_rects.clear();
}

// Reconfigure heads on the current screen.
void BaseScreen::updateHeads(HeadMode head_mode) {
    m_heads.clear();

#ifdef XINERAMA
    if (head_mode == Heads_Xinerama) {
        int head_count;
        XineramaScreenInfo *x_heads = XineramaQueryScreens(display(), &head_count);

        m_heads.reserve(head_count);
        for (int i = 0; i < head_count; i++) {
            XRectangle head = { x_heads[i].x_org, x_heads[i].y_org, x_heads[i].width, x_heads[i].height };
            m_heads.push_back(head);
        }

        if (x_heads) {
            XFree(x_heads);
        }
    } else
#endif  // XINERAMA

    if (head_mode == Heads_One) {
        XRectangle head = { 0, 0, rootWindow().width(), rootWindow().height() };
        m_heads.push_back(head);
    } else {
        throw InitException("Unknown screen head mode given.");
    }
}


// Notifies the screen of the background change.
void BaseScreen::setRootPixmapChanged() {
    damageWholeWindowArea(&m_root_window);

    BasePlugin *plugin = NULL;
    forEachPlugin(i, plugin) {
        plugin->setRootPixmapChanged();
    }
}

// Notifies the screen of a root window change.
void BaseScreen::setRootWindowSizeChanged() {
    damageWholeWindowArea(&m_root_window);

    BasePlugin *plugin = NULL;
    forEachPlugin(i, plugin) {
        plugin->setRootWindowSizeChanged();
    }
}


//--- ACCESSORS ----------------------------------------------------------------

// Returns the damaged screen area.
XserverRegion BaseScreen::damagedScreenArea() {
    XFixesSetRegion(display(), m_screen_damage, (XRectangle*)(m_damaged_screen_rects.data()), m_damaged_screen_rects.size());
    return m_screen_damage;
}


//--- PROPERTY UPDATE FUNCTIONS ------------------------------------------------

// Update stored active window.
void BaseScreen::updateActiveWindow() {
    Window active_window = m_root_window.singlePropertyValue<Window>(Atoms::activeWindowAtom(), None);
    std::list<BaseCompWindow*>::iterator it = getFirstManagedAncestorIterator(active_window);

    if (it != m_windows.end()) {
        m_active_window_xid = (*it)->window();
    } else {
        m_active_window_xid = None;
    }
}

// Update the current iconbar item.
void BaseScreen::updateCurrentIconbarItem() {
    Window current_item = m_root_window.singlePropertyValue<Window>(Atoms::currentIconbarItemAtom(), None);
    std::list<BaseCompWindow*>::iterator it = getFirstManagedAncestorIterator(current_item);

    if (it != m_windows.end()) {
        m_current_iconbar_item = (*it)->window();
    } else {
        m_current_iconbar_item = None;
    }
}

// Update the current workspace index.
void BaseScreen::updateCurrentWorkspace() {
    m_current_workspace = m_root_window.singlePropertyValue<long>(Atoms::workspaceAtom(), 0);
}

// Update stored reconfigure rectangle.
void BaseScreen::updateReconfigureRect() {
    std::vector<long> data = m_root_window.propertyValue<long>(Atoms::reconfigureRectAtom());

    if (data.size() != 4) {
        m_reconfigure_rect.x = m_reconfigure_rect.y = m_reconfigure_rect.width = m_reconfigure_rect.height = 0;
    } else {
        m_reconfigure_rect.x = data[0];
        m_reconfigure_rect.y = data[1];
        m_reconfigure_rect.width = data[2];
        m_reconfigure_rect.height = data[3];
    }
}

// Update stored root window pixmap.
void BaseScreen::updateRootWindowPixmap(Pixmap new_pixmap) {
    if (m_root_window_pixmap && !m_wm_set_root_window_pixmap) {
        XFreePixmap(display(), m_root_window_pixmap);
        m_root_window_pixmap = None;
    }

    if (!new_pixmap) {
        m_root_window_pixmap = rootWindow().firstSinglePropertyValue<Pixmap>(Atoms::rootPixmapAtoms(), None);
    } else {
        m_root_window_pixmap = new_pixmap;
    }
    m_wm_set_root_window_pixmap = true;

    if (!m_root_window_pixmap) {
        fbLog_info << "Cannot find background pixmap, using plain black." << std::endl;
        m_root_window_pixmap = createSolidPixmap(*this, rootWindow().width(), rootWindow().height(), 0x00000000);
        m_wm_set_root_window_pixmap = false;
    }
}

// Update the number of workspaces.
void BaseScreen::updateWorkspaceCount() {
    m_workspace_count = m_root_window.singlePropertyValue<long>(Atoms::workspaceCountAtom(), 1);
}


//--- SCREEN DAMAGE FUNCTIONS --------------------------------------------------

// Damages the reconfigure rectangle on the screen.
void BaseScreen::damageReconfigureRect() {
    damageScreenArea(m_reconfigure_rect);
}

// Damages the given rectangle on the screen.
void BaseScreen::damageScreenArea(XRectangle area) {
    area.height = std::min(area.height + 1, static_cast<int>(rootWindow().height()));
    area.width = std::min(area.width + 1, static_cast<int>(rootWindow().width()));
    m_damaged_screen_rects.push_back(area);
}

// Damages the area in the given window.
void BaseScreen::damageWindowArea(BaseCompWindow *window, XRectangle area) {
    area.x += window->x();
    area.y += window->y();
    damageScreenArea(area);
}

// Damages the area taken by the given window.
void BaseScreen::damageWholeWindowArea(BaseCompWindow *window) {
    XRectangle area = { window->x(), window->y(), window->realWidth() + 2, window->realHeight() + 2 };
    area.height = std::min(area.height, static_cast<short unsigned int>(rootWindow().height()));
    area.width = std::min(area.width, static_cast<short unsigned int>(rootWindow().width()));
    m_damaged_screen_rects.push_back(area);
}


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Returns the parent of a given window.
Window BaseScreen::getParentWindow(Window window) {
    Window root, parent;
    Window *children = 0;
    unsigned int child_count;

    XQueryTree(display(), window, &root, &parent, &children, &child_count);
    if (children) {
        XFree(children);
    }

    return parent;
}

// Returns an iterator of m_windows that points to the given window.
std::list<BaseCompWindow*>::iterator BaseScreen::getWindowIterator(Window window) {
    std::list<BaseCompWindow*>::iterator it = m_windows.begin();
    while (it != m_windows.end()) {
        if (window == (*it)->window()) {
            break;
        }
        ++it;
    }
    return it;
}

// Returns the first managed ancestor of a window.
std::list<BaseCompWindow*>::iterator BaseScreen::getFirstManagedAncestorIterator(Window window) {
    if (window == None) {
        return m_windows.end();
    }

    Window current_window = window;
    std::list<BaseCompWindow*>::iterator it = getWindowIterator(window);

    while (it == m_windows.end()) {
        current_window = getParentWindow(current_window);
        if ((current_window == None) || (current_window == rootWindow().window())) {
            return m_windows.end();
        }
        it = getWindowIterator(current_window);
    }
    return it;
}

// Returns whether the given window is in the ignore list.
bool BaseScreen::isWindowIgnored(Window window) {
    return (find(m_ignore_list.begin(), m_ignore_list.end(), window) != m_ignore_list.end());
}

// Puts a window to a new location on the stack.
void BaseScreen::restackWindow(std::list<BaseCompWindow*>::iterator &window_it, Window above) {
    BaseCompWindow* window = *window_it;
    m_windows.erase(window_it);

    std::list<BaseCompWindow*>::iterator it = getFirstManagedAncestorIterator(above);
    if (it != m_windows.end()) {
        ++it;
        window_it = m_windows.insert(it, window);
    } else {    // Window is just above root.
        m_windows.push_front(window);
        window_it = m_windows.begin();
    }
}


//--- FRIEND OPERATORS -------------------------------------------------

// << output stream operator for the BaseScreen class.
std::ostream &FbCompositor::operator<<(std::ostream& out, const BaseScreen& s) {
    out << "SCREEN NUMBER " << std::dec << s.m_screen_number << ":" << std::endl
        << "  Properties" << std::endl
        << "    Active window XID: " << std::hex << s.m_active_window_xid << std::endl
        << "    Number of workspaces: " << std::dec << s.m_workspace_count << std::endl
        << "    Current workspace: " << std::dec << s.m_current_workspace << std::endl
        << "  Windows" << std::endl;

    std::list<BaseCompWindow*>::const_iterator it = s.m_windows.begin();
    while (it != s.m_windows.end()) {
        out << "    " << **it << std::endl;
        ++it;
    }

    out << "  Ignore list" << std::endl << "    ";
    std::vector<Window>::const_iterator it2 = s.m_ignore_list.begin();
    while (it2 != s.m_ignore_list.end()) {
        out << std::hex << *it2 << " ";
        ++it2;
    }
    out << std::endl;

    return out;
}
