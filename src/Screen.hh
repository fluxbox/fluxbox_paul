// Screen.hh for Fluxbox Window Manager
// Copyright (c) 2001 - 2006 Henrik Kinnunen (fluxgen at fluxbox dot org)
//
// Screen.hh for Blackbox - an X11 Window manager
// Copyright (c) 1997 - 2000 Brad Hughes (bhughes at tcac.net)
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

#ifndef SCREEN_HH
#define SCREEN_HH

#include "FbMenu.hh"
#include "FbWinFrame.hh"
#include "FbRootWindow.hh"
#include "RootTheme.hh"
#include "WinButtonTheme.hh"
#include "FbWinFrameTheme.hh"
#include "TooltipWindow.hh"

#include "FbTk/MenuTheme.hh"
#include "FbTk/EventHandler.hh"
#include "FbTk/Resource.hh"
#include "FbTk/MultLayers.hh"
#include "FbTk/NotCopyable.hh"
#include "FbTk/Signal.hh"

#include "FocusControl.hh"

#include <X11/Xresource.h>

#ifdef HAVE_CSTDIO
  #include <cstdio>
#else
  #include <stdio.h>
#endif
#include <list>
#include <vector>
#include <fstream>
#include <memory>
#include <map>

class ClientPattern;
class Focusable;
class FluxboxWindow;
class WinClient;
class Workspace;
class Strut;
class Slit;
class Toolbar;
class HeadArea;
class ScreenPlacement;
class TooltipWindow;
class OSDWindow;

namespace FbTk {
class Menu;
class ImageControl;
class LayerItem;
class FbWindow;
}


/// Handles screen connection, screen clients and workspaces
/**
 Create workspaces, handles switching between workspaces and windows
 */
class BScreen: public FbTk::EventHandler, private FbTk::NotCopyable {
public:
    typedef std::list<FluxboxWindow *> Icons;

    typedef std::vector<Workspace *> Workspaces;
    typedef std::vector<std::string> WorkspaceNames;
    typedef std::list<std::pair<FbTk::FbString, FbTk::RefCount<FbTk::Menu> > > ExtraMenus;

    BScreen(FbTk::ResourceManager_base &rm,
            const std::string &screenname,
            int scrn, int number_of_layers);
    ~BScreen();

    void initWindows();
    void initMenus();

    bool isRootColormapInstalled() const { return root_colormap_installed; }
    bool isScreenManaged() const { return managed; }
    bool isWorkspaceWarping() const { return (m_workspaces_list.size() > 1) && *resource.workspace_warping; }
    bool doAutoRaise() const { return *resource.auto_raise; }
    bool clickRaises() const { return *resource.click_raises; }
    bool doOpaqueMove() const { return *resource.opaque_move; }
    bool doFullMax() const { return *resource.full_max; }
    bool getMaxIgnoreIncrement() const { return *resource.max_ignore_inc; }
    bool getMaxDisableMove() const { return *resource.max_disable_move; }
    bool getMaxDisableResize() const { return *resource.max_disable_resize; }
    bool doShowWindowPos() const { return *resource.show_window_pos; }
    const std::string &defaultDeco() const { return *resource.default_deco; }
    const std::string windowMenuFilename() const;
    FbTk::ImageControl &imageControl() { return *m_image_control.get(); }
    // menus
    const FbMenu &rootMenu() const { return *m_rootmenu.get(); }
    FbMenu &rootMenu() { return *m_rootmenu.get(); }
    FbTk::RefCount<const FbMenu> configMenu() const { return m_configmenu; }
    const FbTk::RefCount<FbMenu> &configMenu() { return m_configmenu; }
    const FbMenu &windowMenu() const { return *m_windowmenu.get(); }
    FbMenu &windowMenu() { return *m_windowmenu.get(); }
    ExtraMenus &extraWindowMenus() { return m_extramenus; }
    const ExtraMenus &extraWindowMenus() const { return m_extramenus; }

    FbWinFrame::TabPlacement getTabPlacement() const { return *resource.tab_placement; }

    unsigned int noFocusWhileTypingDelay() const { return *resource.typing_delay; }
    const bool allowRemoteActions() const { return *resource.allow_remote_actions; }
    const bool clientMenuUsePixmap() const { return *resource.clientmenu_use_pixmap; }
    const bool getDefaultInternalTabs() const { return *resource.default_internal_tabs; }
    const bool getTabsUsePixmap() const { return *resource.tabs_use_pixmap; }
    const bool getMaxOverTabs() const { return *resource.max_over_tabs; }

    unsigned int getTabWidth() const { return *resource.tab_width; }
    /// @return the slit, @see Slit
    Slit *slit() { return m_slit.get(); }
    /// @return the slit, @see Slit
    const Slit *slit() const { return m_slit.get(); }
    /**
     * @param w the workspace number
     * @return workspace for the given workspace number
     */
    Workspace *getWorkspace(unsigned int w) { return ( w < m_workspaces_list.size() ? m_workspaces_list[w] : 0); }
    /**
     * @param w the workspace number
     * @return workspace for the given workspace number
     */
    const Workspace *getWorkspace(unsigned int w) const {
        return (w < m_workspaces_list.size() ? m_workspaces_list[w] : 0);
    }
    /// @return the current workspace
    Workspace *currentWorkspace() { return m_current_workspace; }
    const Workspace *currentWorkspace() const { return m_current_workspace; }
    /// @return the workspace menu
    FbTk::RefCount<const FbMenu> workspaceMenu() const { return m_workspacemenu; }
    /// @return the workspace menu
    const FbTk::RefCount<FbMenu> &workspaceMenu() { return m_workspacemenu; }
    /// @return focus control handler
    const FocusControl &focusControl() const { return *m_focus_control; }
    /// @return focus control handler
    FocusControl &focusControl() { return *m_focus_control; }
    /// @return the current workspace id
    unsigned int currentWorkspaceID() const;
    /**

     *
    */
    /// @return maximum screen bound to the left for a specific xinerama head
    unsigned int maxLeft(int head) const;
    /// @return maximum screen bound to the right for a specific xinerama head
    unsigned int maxRight(int head) const;
    /// @return maximum screen bound at the top for the specified xinerama head
    unsigned int maxTop(int head) const;
     /// @return maximum screen bound at bottom for the specified xinerama head
    unsigned int maxBottom(int head) const;
    /// @return true if window is kde dock app
    bool isKdeDockapp(Window win) const;
    /// @return true if dock app was added, else false
    bool addKdeDockapp(Window win);
    /// @return screen width, @see rootWindow()
    unsigned int width() const { return rootWindow().width(); }
    /// @return screen height, @see rootWindow()
    unsigned int height() const { return rootWindow().height(); }
    /// @return number of the screen, @see rootWindow()
    int screenNumber() const { return rootWindow().screenNumber(); }

    /// @return number of workspaces
    size_t numberOfWorkspaces() const { return m_workspaces_list.size(); }

    const Icons &iconList() const { return m_icon_list; }
    Icons &iconList() { return m_icon_list; }

    const Workspaces &getWorkspacesList() const { return m_workspaces_list; }
    Workspaces &getWorkspacesList() { return m_workspaces_list; }
    const WorkspaceNames &getWorkspaceNames() const { return *resource.workspace_names; }
    /**
       @name Screen signals
    */
    //@{
    typedef FbTk::Signal<BScreen&> ScreenSignal;
    /// client list signal
    ScreenSignal &clientListSig() { return m_clientlist_sig; }
    /// icon list sig
    ScreenSignal &iconListSig() { return m_iconlist_sig; }
    /// workspace count signal
    ScreenSignal &workspaceCountSig() { return m_workspacecount_sig; }
    /// workspace names signal
    ScreenSignal &workspaceNamesSig() { return m_workspacenames_sig; }
    /// workspace area signal
    ScreenSignal &workspaceAreaSig() { return m_workspace_area_sig; }
    /// current workspace signal
    ScreenSignal &currentWorkspaceSig() { return m_currentworkspace_sig; }
    /// focused window signal
    FbTk::Signal<BScreen&, FluxboxWindow*, WinClient*> &focusedWindowSig() { return m_focusedwindow_sig; }
    /// reconfigure signal
    ScreenSignal &reconfigureSig() { return m_reconfigure_sig; }
    ScreenSignal &resizeSig() { return m_resize_sig; }
    ScreenSignal &bgChangeSig() { return m_bg_change_sig; }
    //@}

    void propertyNotify(Atom atom);
    void keyPressEvent(XKeyEvent &ke);
    void keyReleaseEvent(XKeyEvent &ke);
    void buttonPressEvent(XButtonEvent &be);

    /**
     * Cycles focus of windows
     * @param opts focus options
     * @param pat specific pattern to match windows with
     * @param reverse the order of cycling
     */
    void cycleFocus(int opts = 0, const ClientPattern *pat = 0, bool reverse = false);

    bool isCycling() const { return m_cycling; }

    /**
     * Creates an empty menu with specified label
     * @param label for the menu
     * @return created menu
     */
    FbMenu *createMenu(const std::string &label);
    /**
     * Creates an empty toggle menu with a specific label
     * @param label
     * @return created menu
     */
    FbMenu *createToggleMenu(const std::string &label);

    /**
     * For extras to add menus.
     */
    void addExtraWindowMenu(const FbTk::FbString &label, const FbTk::RefCount<FbTk::Menu> &menu);

    int getEdgeSnapThreshold() const { return *resource.edge_snap_threshold; }

    void setRootColormapInstalled(bool r) { root_colormap_installed = r;  }

    void saveTabPlacement(FbWinFrame::TabPlacement place) { *resource.tab_placement = place; }

    void saveWorkspaces(int w) { *resource.workspaces = w;  }

    FbTk::ThemeProxy<FbWinFrameTheme> &focusedWinFrameTheme() { return *m_focused_windowtheme.get(); }
    const FbTk::ThemeProxy<FbWinFrameTheme> &focusedWinFrameTheme() const { return *m_focused_windowtheme.get(); }
    FbTk::ThemeProxy<FbWinFrameTheme> &unfocusedWinFrameTheme() { return *m_unfocused_windowtheme.get(); }
    const FbTk::ThemeProxy<FbWinFrameTheme> &unfocusedWinFrameTheme() const { return *m_unfocused_windowtheme.get(); }

    FbTk::ThemeProxy<FbTk::MenuTheme> &menuTheme() { return *m_menutheme.get(); }
    const FbTk::ThemeProxy<FbTk::MenuTheme> &menuTheme() const { return *m_menutheme.get(); }
    const FbTk::ThemeProxy<RootTheme> &rootTheme() const { return *m_root_theme.get(); }

    FbTk::ThemeProxy<WinButtonTheme> &focusedWinButtonTheme() { return *m_focused_winbutton_theme.get(); }
    const FbTk::ThemeProxy<WinButtonTheme> &focusedWinButtonTheme() const { return *m_focused_winbutton_theme.get(); }
    FbTk::ThemeProxy<WinButtonTheme> &unfocusedWinButtonTheme() { return *m_unfocused_winbutton_theme.get(); }
    const FbTk::ThemeProxy<WinButtonTheme> &unfocusedWinButtonTheme() const { return *m_unfocused_winbutton_theme.get(); }
    FbTk::ThemeProxy<WinButtonTheme> &pressedWinButtonTheme() { return *m_pressed_winbutton_theme.get(); }
    const FbTk::ThemeProxy<WinButtonTheme> &pressedWinButtonTheme() const { return *m_pressed_winbutton_theme.get(); }

    FbRootWindow &rootWindow() { return m_root_window; }
    const FbRootWindow &rootWindow() const { return m_root_window; }

    FbTk::FbWindow &dummyWindow() { return m_dummy_window; }
    const FbTk::FbWindow &dummyWindow() const { return m_dummy_window; }

    FbTk::MultLayers &layerManager() { return m_layermanager; }
    const FbTk::MultLayers &layerManager() const { return m_layermanager; }
    FbTk::ResourceManager_base &resourceManager() { return m_resource_manager; }
    const FbTk::ResourceManager_base &resourceManager() const { return m_resource_manager; }
    const std::string &name() const { return m_name; }
    bool isShuttingdown() const { return m_shutdown; }
    bool isRestart();

    ScreenPlacement &placementStrategy() { return *m_placement_strategy; }
    const ScreenPlacement &placementStrategy() const { return *m_placement_strategy; }

    int addWorkspace();
    int removeLastWorkspace();
    // scroll workspaces
    /**
     * Jump forward to a workspace
     * @param delta number of steps to jump
     */
    void nextWorkspace(int delta = 1);
    /**
     * Jump backwards to a workspace
     * @param delta number of steps to jump
     */
    void prevWorkspace(int delta = 1);
    /**
     * Jump right to a workspace.
     * @param delta number of steps to jump
     */
    void rightWorkspace(int delta);
    /**
     * Jump left to a workspace
     * @param delta number of steps to jump
     */
    void leftWorkspace(int delta);

    /// update workspace name for given workspace
    void updateWorkspaceName(unsigned int w);
    /// remove all workspace names
    void removeWorkspaceNames();
    /// add a workspace name to the end of the workspace name list
    void addWorkspaceName(const std::string &name);
    /// add a window to the icon list
    void addIcon(FluxboxWindow *win);
    /// remove a window from the icon list
    void removeIcon(FluxboxWindow *win);
    /// remove a window
    void removeWindow(FluxboxWindow *win);
    /// remove a client
    void removeClient(WinClient &client);
    /**
     * Gets name of a specific workspace
     * @param workspace the workspace number to get the name of
     * @return name of the workspace
     */
    std::string getNameOfWorkspace(unsigned int workspace) const;
    /// changes workspace to specified id
    void changeWorkspaceID(unsigned int, bool revert = true);
    /**
     * Sends a window to a workspace
     * @param workspace the workspace id
     * @param win the window to send
     * @param changeworkspace whether current workspace should change
     */
    void sendToWorkspace(unsigned int workspace, FluxboxWindow *win=0,
                         bool changeworkspace=true);
    /**
     * Reassociate a window to another workspace
     * @param window the window to reassociate
     * @param workspace_id id of the workspace
     * @param ignore_sticky ignores any sticky windows
     */
    void reassociateWindow(FluxboxWindow *window, unsigned int workspace_id,
                           bool ignore_sticky);


    void reconfigure();
    void reconfigureTabs();
    void rereadMenu();
    void rereadWindowMenu();
    void shutdown();
    /// show position window centered on the screen with "X x Y" text
    void showPosition(int x, int y);
    void hidePosition();
    /// show geomentry with "width x height"-text, not size of window
    void showGeometry(unsigned int width, unsigned int height);
    void hideGeometry();

    /// @param text the text to be displayed in the tooltip window
    void showTooltip(const FbTk::BiDiString &text);
    /// Hides the tooltip window
    void hideTooltip();

    TooltipWindow& tooltipWindow() { return *m_tooltip_window; }

    void setLayer(FbTk::LayerItem &item, int layernum);
    // remove? no, items are never removed from their layer until they die

    /// updates root window size and resizes/reconfigures screen clients
    /// that depends on screen size (slit)
    /// (and maximized windows?)
    void updateSize();

    // Xinerama-related functions

    /// @return true if xinerama is available
    bool hasXinerama() const { return m_xinerama_avail; }
    /// @return umber of xinerama heads
    int numHeads() const { return m_xinerama_num_heads; }

    void initXinerama();
    void clearHeads();
    /// clean up xinerama
    void clearXinerama();

    /**
     * Determines head number for a position
     * @param x position in pixels on the screen
     * @param y position in pixels on the screen
     * @return head number at this position
     */
    int getHead(int x, int y) const;
    /// @return head number of window
    int getHead(const FbTk::FbWindow &win) const;
    /// @return the current head number
    int getCurrHead() const;
    /// @return head x position
    int getHeadX(int head) const;
    /// @return head y position
    int getHeadY(int head) const;
    /// @return width of the head
    int getHeadWidth(int head) const;
    /// @return height of the head
    int getHeadHeight(int head) const;

    ///  @return the new (x,y) for a rectangle fitted on a head
    std::pair<int,int> clampToHead(int head, int x, int y, int w, int h) const;

    // magic to allow us to have "on head" placement (menu) without
    // the object really knowing about it.
    template <typename OnHeadObject>
    int getOnHead(OnHeadObject &obj) const;

    // grouping - we want ordering, so we can either search for a
    // group to the left, or to the right (they'll be different if
    // they exist).
    WinClient *findGroupLeft(WinClient &winclient);
    WinClient *findGroupRight(WinClient &winclient);

    /// create window frame for client window and attach it
    FluxboxWindow *createWindow(Window clientwin);
    /// creates a window frame for a winclient. The client is attached to the window
    FluxboxWindow *createWindow(WinClient &client);
    /// request workspace space, i.e "don't maximize over this area"
    Strut *requestStrut(int head, int left, int right, int top, int bottom);
    /// remove requested space and destroy strut
    void clearStrut(Strut *strut);
    /// updates max avaible area for the workspace
    void updateAvailableWorkspaceArea();

    // for extras to add menus. These menus must be marked
    // internal for their safety, and __the extension__ must
    // delete and remove the menu itself (opposite to Window)
    void addConfigMenu(const FbTk::FbString &label, const FbTk::RefCount<FbTk::Menu> &menu);
    void removeConfigMenu(const FbTk::RefCount<FbTk::Menu> &menu);


    /// Adds a resource to managed resource list
    /// This resource is now owned by Screen and will be destroyed
    /// when screen dies
    void addManagedResource(FbTk::Resource_base *resource);

private:
    void setupConfigmenu(FbTk::Menu &menu);
    void renderGeomWindow();
    void renderPosWindow();
    void focusedWinFrameThemeReconfigured();

    const Strut* availableWorkspaceArea(int head) const;

    FbTk::SignalTracker m_tracker;
    ScreenSignal m_reconfigure_sig; ///< reconfigure signal

    FbTk::Signal<BScreen&, FluxboxWindow*, WinClient*> m_focusedwindow_sig;  ///< focused window signal
    ScreenSignal m_resize_sig; ///< resize signal
    ScreenSignal m_workspace_area_sig; ///< workspace area changed signal
    ScreenSignal m_iconlist_sig; ///< notify if a window gets iconified/deiconified
    ScreenSignal m_clientlist_sig;  ///< client signal
    ScreenSignal m_bg_change_sig; ///< background change signal
    ScreenSignal m_workspacecount_sig; ///< workspace count signal
    ScreenSignal m_currentworkspace_sig; ///< current workspace signal
    ScreenSignal m_workspacenames_sig; ///< workspace names signal

    FbTk::MultLayers m_layermanager;

    bool root_colormap_installed, managed;

    std::auto_ptr<FbTk::ImageControl> m_image_control;
    FbTk::RefCount<FbMenu> m_configmenu, m_rootmenu, m_workspacemenu, m_windowmenu;

    ExtraMenus m_extramenus;

    typedef std::list<std::pair<FbTk::FbString, FbTk::RefCount<FbTk::Menu> > > Configmenus;


    Configmenus m_configmenu_list;
    Icons m_icon_list;

    std::auto_ptr<Slit> m_slit;
    std::auto_ptr<Toolbar> m_toolbar;

    Workspace *m_current_workspace;

    Workspaces m_workspaces_list;

    std::auto_ptr<FbWinFrameTheme> m_focused_windowtheme,
                                   m_unfocused_windowtheme;
    std::auto_ptr<WinButtonTheme> m_focused_winbutton_theme,
            m_unfocused_winbutton_theme, m_pressed_winbutton_theme;
    std::auto_ptr<FbTk::MenuTheme> m_menutheme;
    std::auto_ptr<RootTheme> m_root_theme;

    FbRootWindow m_root_window;
    std::auto_ptr<OSDWindow> m_geom_window, m_pos_window;
    std::auto_ptr<TooltipWindow> m_tooltip_window;
    FbTk::FbWindow m_dummy_window;

    struct ScreenResource {
        ScreenResource(FbTk::ResourceManager_base &rm, const std::string &scrname);

        FbTk::Resource<
                std::vector<std::string>, FbTk::VectorTraits<FbTk::StringTraits>
        > workspace_names;

        FbTk::BoolResource opaque_move, full_max,
            max_ignore_inc, max_disable_move, max_disable_resize,
            workspace_warping, show_window_pos, auto_raise, click_raises;
        FbTk::StringResource default_deco;
        FbTk::Resource<FbWinFrame::TabPlacement, FbTk::EnumTraits<FbWinFrame::TabPlacement> > tab_placement;
        FbTk::StringResource windowmenufile;
        FbTk::UIntResource typing_delay;
        FbTk::IntResource workspaces, edge_snap_threshold, focused_alpha,
            unfocused_alpha, menu_alpha;
        FbTk::RangedIntResource menu_delay;
        FbTk::IntResource  tab_width, tooltip_delay;
        FbTk::BoolResource allow_remote_actions;
        FbTk::BoolResource clientmenu_use_pixmap;
        FbTk::BoolResource tabs_use_pixmap;
        FbTk::BoolResource max_over_tabs;
        FbTk::BoolResource default_internal_tabs;


    } resource;

    /// Holds manage resources that screen destroys
    FbTk::ResourceManager::ResourceList m_managed_resources;

    FbTk::ResourceManager_base &m_resource_manager;
    const std::string m_name;

    FocusControl *m_focus_control;
    ScreenPlacement *m_placement_strategy;

    // This is a map of windows to clients for clients that had a left
    // window set, but that window wasn't present at the time
    typedef std::map<Window, WinClient *> Groupables;
    Groupables m_expecting_groups;

    bool m_cycling;
    const ClientPattern *m_cycle_opts;

    // Xinerama related private data
    bool m_xinerama_avail;
    int m_xinerama_num_heads;
    int m_xinerama_center_x, m_xinerama_center_y;

    std::vector<HeadArea *> m_head_areas;

    struct XineramaHeadInfo {
        int _x, _y, _width, _height;
        int x() const { return _x; }
        int y() const { return _y; }
        int width() const { return _width; }
        int height() const { return _height; }
    } *m_xinerama_headinfo;

    bool m_restart, m_shutdown;
};


#endif // SCREEN_HH
