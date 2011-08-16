// fluxbox-update_configs.cc
// Copyright (c) 2007 Fluxbox Team (fluxgen at fluxbox dot org)
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

#include "../src/FbTk/Container.hh"
#include "../src/FbTk/I18n.hh"
#include "../src/FbTk/LResource.hh"
#include "../src/FbTk/LuaUtil.hh"
#include "../src/FbTk/StringUtil.hh"
#include "../src/FbTk/FileUtil.hh"

#include "../src/defaults.hh"
#include "../src/Resources.hh"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif // HAVE_SIGNAL_H

//use GNU extensions
#ifndef         _GNU_SOURCE
#define         _GNU_SOURCE
#endif // _GNU_SOURCE

#ifdef HAVE_CSTRING
  #include <cstring>
#else
  #include <string.h>
#endif

#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <cstdlib>
#include <list>
#include <vector>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ifstream;
using std::ofstream;
using std::set;
using std::map;
using std::list;
using std::exit;
using std::getenv;

string read_file(const string& filename);
void write_file(const string& filename, const string &contents);
void save_all_files();


/*------------------------------------------------------------------*\
\*------------------------------------------------------------------*/

void update_add_mouse_evens_to_keys(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {
    string keyfilename = FbTk::StringUtil::expandFilename(rm->resourceValue("keyFile"));

    string whole_keyfile = read_file(keyfilename);
    string new_keyfile = "";
    // let's put our new keybindings first, so they're easy to find
    new_keyfile += "!mouse actions added by fluxbox-update_configs\n";
    new_keyfile += "OnDesktop Mouse1 :HideMenus\n";
    new_keyfile += "OnDesktop Mouse2 :WorkspaceMenu\n";
    new_keyfile += "OnDesktop Mouse3 :RootMenu\n";

    // scrolling on desktop needs to match user's desktop wheeling settings
    // hmmm, what are the odds that somebody wants this to be different on
    // different screens? the ability is going away until we make per-screen
    // keys files, anyway, so let's just use the first screen's setting
    FbTk::BoolResource rc_wheeling(*rm, true,
            "screen0.desktopwheeling",
            "Screen0.DesktopWheeling");
    FbTk::BoolResource rc_reverse(*rm, false,
            "screen0.reversewheeling",
            "Screen0.ReverseWheeling");
    if (*rc_wheeling) {
        if (*rc_reverse) { // if you ask me, this should have been default
            new_keyfile += "OnDesktop Mouse4 :PrevWorkspace\n";
            new_keyfile += "OnDesktop Mouse5 :NextWorkspace\n";
        } else {
            new_keyfile += "OnDesktop Mouse4 :NextWorkspace\n";
            new_keyfile += "OnDesktop Mouse5 :PrevWorkspace\n";
        }
    }
    new_keyfile += "\n"; // just for good looks
    new_keyfile += whole_keyfile; // don't forget user's old keybindings

    write_file(keyfilename, new_keyfile);
}


void update_move_groups_entries_to_apps_file(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {

    string appsfilename = FbTk::StringUtil::expandFilename(rm->resourceValue("appsFile"));
    FbTk::StringResource rc_groupfile(*rm, "~/.fluxbox/groups",
            "groupFile", "GroupFile");
    string groupfilename = FbTk::StringUtil::expandFilename(*rc_groupfile);
    string whole_groupfile = read_file(groupfilename);
    string whole_appsfile = read_file(appsfilename);
    string new_appsfile = "";

    list<string> lines;
    FbTk::StringUtil::stringtok(lines, whole_groupfile, "\n");

    list<string>::iterator line_it = lines.begin();
    list<string>::iterator line_it_end = lines.end();
    for (; line_it != line_it_end; ++line_it) {
        new_appsfile += "[group] (workspace=[current])\n";

        list<string> apps;
        FbTk::StringUtil::stringtok(apps, *line_it);

        list<string>::iterator it = apps.begin();
        list<string>::iterator it_end = apps.end();
        for (; it != it_end; ++it) {
            new_appsfile += " [app] (name=";
            new_appsfile += *it;
            new_appsfile += ")\n";
        }

        new_appsfile += "[end]\n";
    }

    new_appsfile += whole_appsfile;
    write_file(appsfilename, new_appsfile);
}


void update_move_toolbar_wheeling_to_keys_file(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {

    string keyfilename = FbTk::StringUtil::expandFilename(rm->resourceValue("keyFile"));
    string whole_keyfile = read_file(keyfilename);
    string new_keyfile = "";
    // let's put our new keybindings first, so they're easy to find
    new_keyfile += "!mouse actions added by fluxbox-update_configs\n";
    bool keep_changes = false;

    // scrolling on toolbar needs to match user's toolbar wheeling settings
    FbTk::StringResource rc_wheeling(*rm, "Off",
            "screen0.iconbar.wheelMode",
            "Screen0.Iconbar.WheelMode");
    FbTk::BoolResource rc_screen(*rm, true,
            "screen0.desktopwheeling",
            "Screen0.DesktopWheeling");
    FbTk::BoolResource rc_reverse(*rm, false,
            "screen0.reversewheeling",
            "Screen0.ReverseWheeling");
    if (strcasecmp(rc_wheeling->c_str(), "On") == 0 ||
            (strcasecmp(rc_wheeling->c_str(), "Screen") == 0 && *rc_screen)) {
        keep_changes = true;
        if (*rc_reverse) { // if you ask me, this should have been default
            new_keyfile += "OnToolbar Mouse4 :PrevWorkspace\n";
            new_keyfile += "OnToolbar Mouse5 :NextWorkspace\n";
        } else {
            new_keyfile += "OnToolbar Mouse4 :NextWorkspace\n";
            new_keyfile += "OnToolbar Mouse5 :PrevWorkspace\n";
        }
    }
    new_keyfile += "\n"; // just for good looks
    new_keyfile += whole_keyfile; // don't forget user's old keybindings

    if (keep_changes)
        write_file(keyfilename, new_keyfile);
}



void update_move_modkey_to_keys_file(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {
    string keyfilename = FbTk::StringUtil::expandFilename(rm->resourceValue("keyFile"));
    string whole_keyfile = read_file(keyfilename);
    string new_keyfile = "";
    // let's put our new keybindings first, so they're easy to find
    new_keyfile += "!mouse actions added by fluxbox-update_configs\n";

    // need to match user's resize model
    FbTk::StringResource rc_mode(*rm, "Bottom",
            "screen0.resizeMode",
            "Screen0.ResizeMode");
    FbTk::StringResource rc_modkey(*rm, "Mod1",
            "modKey",
            "ModKey");

    new_keyfile += "OnWindow " + rc_modkey.get() +
        " Mouse1 :MacroCmd {Raise} {Focus} {StartMoving}\n";
    new_keyfile += "OnWindow " + rc_modkey.get() +
        " Mouse3 :MacroCmd {Raise} {Focus} {StartResizing ";
    if (strcasecmp(rc_mode->c_str(), "Quadrant") == 0) {
        new_keyfile += "NearestCorner}\n";
    } else if (strcasecmp(rc_mode->c_str(), "Center") == 0) {
        new_keyfile += "Center}\n";
    } else {
        new_keyfile += "BottomRight}\n";
    }
    new_keyfile += "\n"; // just for good looks
    new_keyfile += whole_keyfile; // don't forget user's old keybindings

    write_file(keyfilename, new_keyfile);
}




void
update_window_patterns_for_iconbar(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {

    FbTk::StringResource &rc_mode =
            rm->getResource<string, FbTk::StringTraits>("screen0.iconbar.mode");

    std::string mode = FbTk::StringUtil::toLower(*rc_mode);
    if (mode == "none")
        rc_mode = "none";
    else if (mode == "icons")
        rc_mode = "{static groups} (minimized=yes)";
    else if (mode == "noicons")
        rc_mode = "{static groups} (minimized=no)";
    else if (mode == "workspaceicons")
        rc_mode = "{static groups} (minimized=yes) (workspace)";
    else if (mode == "workspacenoicons")
        rc_mode = "{static groups} (minimized=no) (workspace)";
    else if (mode == "allwindows")
        rc_mode = "{static groups}";
    else
        rc_mode = "{static groups} (workspace)";

}


void update_move_titlebar_actions_to_keys_file(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {
    string keyfilename = FbTk::StringUtil::expandFilename(rm->resourceValue("keyFile"));
    string whole_keyfile = read_file(keyfilename);
    string new_keyfile = "";
    // let's put our new keybindings first, so they're easy to find
    new_keyfile += "!mouse actions added by fluxbox-update_configs\n";
    new_keyfile += "OnTitlebar Double Mouse1 :Shade\n";
    new_keyfile += "OnTitlebar Mouse3 :WindowMenu\n";

    FbTk::BoolResource rc_reverse(*rm, false,"screen0.reversewheeling", "Screen0.ReverseWheeling");
    FbTk::StringResource  scroll_action(*rm, "", "screen0.windowScrollAction", "Screen0.WindowScrollAction");
    if (strcasecmp(scroll_action->c_str(), "shade") == 0) {
        if (*rc_reverse) {
            new_keyfile += "OnTitlebar Mouse5 :ShadeOn\n";
            new_keyfile += "OnTitlebar Mouse4 :ShadeOff\n";
        } else {
            new_keyfile += "OnTitlebar Mouse4 :ShadeOn\n";
            new_keyfile += "OnTitlebar Mouse5 :ShadeOff\n";
        }
    } else if (strcasecmp(scroll_action->c_str(), "nexttab") == 0) {
        if (*rc_reverse) {
            new_keyfile += "OnTitlebar Mouse5 :PrevTab\n";
            new_keyfile += "OnTitlebar Mouse4 :NextTab\n";
        } else {
            new_keyfile += "OnTitlebar Mouse4 :PrevTab\n";
            new_keyfile += "OnTitlebar Mouse5 :NextTab\n";
        }
    }

    new_keyfile += "\n"; // just for good looks
    new_keyfile += whole_keyfile; // don't forget user's old keybindings

    write_file(keyfilename, new_keyfile);

}


void update_added_starttabbing_command(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {
    string keyfilename = FbTk::StringUtil::expandFilename(rm->resourceValue("keyFile"));
    string whole_keyfile = read_file(keyfilename);
    string new_keyfile = "";
    // let's put our new keybindings first, so they're easy to find
    new_keyfile += "!mouse actions added by fluxbox-update_configs\n";
    new_keyfile += "OnTitlebar Mouse2 :StartTabbing\n\n";
    new_keyfile += whole_keyfile; // don't forget user's old keybindings

    write_file(keyfilename, new_keyfile);
}



void
update_disable_icons_in_tabs_for_backwards_compatibility(
        std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {

    FbTk::BoolResource &show =
            rm->getResource<bool, FbTk::BoolTraits>("screen0.tabs.usePixmap");
    if (!show) // only change if the setting didn't already exist
        show = false;
}




void update_change_format_of_split_placement_menu(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {

    PlacementResource &placement =
            rm->getResource<Placement, FbTk::EnumTraits<Placement> >("screen0.slit.placement");

    FbTk::StringResource &direction =
            rm->getResource<string, FbTk::StringTraits>("screen0.slit.direction");

    if (strcasecmp(direction->c_str(), "vertical") == 0) {
        if (*placement == BOTTOMRIGHT)
            placement = RIGHTBOTTOM;
        else if (*placement == BOTTOMLEFT)
            placement = LEFTBOTTOM;
        else if (*placement == TOPRIGHT)
            placement = RIGHTTOP;
        else if (*placement == TOPLEFT)
            placement = LEFTTOP;
    }
}




void update_update_keys_file_for_nextwindow_syntax_changes(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {

    string keyfilename = FbTk::StringUtil::expandFilename(rm->resourceValue("keyFile"));
    string whole_keyfile = read_file(keyfilename);

    size_t pos = 0;
    while (true) {
        const char *keyfile = whole_keyfile.c_str();
        const char *loc = 0;
        size_t old_pos = pos;
        // find the first position that matches any of next/prevwindow/group
        if ((loc = FbTk::StringUtil::strcasestr(keyfile + old_pos,
                        "nextwindow")))
            pos = (loc - keyfile) + 10;
        if ((loc = FbTk::StringUtil::strcasestr(keyfile + old_pos,
                        "prevwindow")))
            pos = (pos > old_pos && keyfile + pos < loc) ?
                pos : (loc - keyfile) + 10;
        if ((loc = FbTk::StringUtil::strcasestr(keyfile + old_pos,
                        "nextgroup")))
            pos = (pos > old_pos && keyfile + pos < loc) ?
                pos : (loc - keyfile) + 9;
        if ((loc = FbTk::StringUtil::strcasestr(keyfile + old_pos,
                        "prevgroup")))
            pos = (pos > old_pos && keyfile + pos < loc) ?
                pos : (loc - keyfile) + 9;
        if (pos == old_pos)
            break;

        pos = whole_keyfile.find_first_not_of(" \t", pos);
        if (pos != std::string::npos && isdigit(whole_keyfile[pos])) {
            char *endptr = 0;
            unsigned int mask = strtoul(keyfile + pos, &endptr, 0);
            string insert = "";
            if ((mask & 9) == 9)
                insert = "{static groups}";
            else if (mask & 1)
                insert = "{groups}";
            else if (mask & 8)
                insert = "{static}";
            if (mask & 2)
                insert += " (stuck=no)";
            if (mask & 4)
                insert += " (shaded=no)";
            if (mask & 16)
                insert += " (minimized=no)";
            if (mask)
                whole_keyfile.replace(pos, endptr - keyfile - pos, insert);
        }
    }

    write_file(keyfilename, whole_keyfile);
}




void update_keys_for_ongrip_onwindowborder(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {

    string keyfilename = FbTk::StringUtil::expandFilename(rm->resourceValue("keyFile"));
    string whole_keyfile = read_file(keyfilename);
    string new_keyfile = "";
    // let's put our new keybindings first, so they're easy to find
    new_keyfile += "!mouse actions added by fluxbox-update_configs\n";
    new_keyfile += "OnTitlebar Move1 :StartMoving\n";
    new_keyfile += "OnLeftGrip Move1 :StartResizing bottomleft\n";
    new_keyfile += "OnRightGrip Move1 :StartResizing bottomright\n";
    new_keyfile += "OnWindowBorder Move1 :StartMoving\n\n";

    new_keyfile += "\n"; // just for good looks
    new_keyfile += whole_keyfile; // don't forget user's old keybindings

    write_file(keyfilename, new_keyfile);
}




void update_keys_for_activetab(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {

    string keyfilename = FbTk::StringUtil::expandFilename(rm->resourceValue("keyFile"));
    string whole_file = read_file(keyfilename);
    string new_keyfile = "";

    new_keyfile += "!mouse actions added by fluxbox-update_configs\n";
    new_keyfile += "OnTitlebar Mouse1 :MacroCmd {Focus} {Raise} {ActivateTab}\n";

    new_keyfile += "\n"; // just for good looks
    new_keyfile += whole_file;

    write_file(keyfilename, new_keyfile);

}



// NextWindow {static groups} => NextWindow {static groups} (workspace=[current])
void update_limit_nextwindow_to_current_workspace(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {

    string keyfilename = FbTk::StringUtil::expandFilename(rm->resourceValue("keyFile"));
    string whole_file = read_file(keyfilename);
    string new_keyfile = "";

    new_keyfile += "! fluxbox-update_configs added '(workspace=[current])' to (Next|Prev)(Window|Group)\n";
    new_keyfile += "! check lines marked by 'FBCV13' if they are correctly updated\n";

    const char* pos = whole_file.c_str();


    string last_word;
    enum { SCAN, COMMENT, CMD, OPEN_STATIC_GROUP_OR_PATTERN, CLOSE_STATIC_GROUP };
    int state = SCAN;
    bool mark_line_change = false;
    for ( ; *pos; ++pos) {

        new_keyfile += *pos;

        char c = tolower(*pos);
        switch (state) {
        case SCAN:
            if (c == '{' || c == ':') // ':NextWindow' or 'MacroCmd {NextWindow'
                state = CMD;
            break;
        case CMD:
            if (isspace(c) || c == '}' || c == '#' || c == '!' ) {
                if (last_word == "nextwindow" || last_word == "nextgroup" ||
                        last_word == "prevwindow" || last_word == "prevgroup") {

                    // ':NewWindow[\n!#]' or 'MacroCmd {NextWindow}'
                    if (c == '\n' || c == '#' || c == '!' || c == '}') {
                        new_keyfile.insert(new_keyfile.size() - 1, " (workspace=[current])");
                        mark_line_change = true;
                        state = SCAN;
                    } else {
                        state = OPEN_STATIC_GROUP_OR_PATTERN;
                    }
                } else {
                    state = SCAN;
                    last_word.clear();
                }
            } else { // possible '(next|prev)(group|window)'
                last_word += c;
            }
            break;
        case OPEN_STATIC_GROUP_OR_PATTERN:
            if (c == '{') { // NextWindow {static group}
                state = CLOSE_STATIC_GROUP;
            } else if (c == '(') { // NextWindow (foo=bar)
                new_keyfile += "workspace=[current]) (";
                mark_line_change = true;
                state = SCAN;
            } else if (c == '}') { // MacroCmd {NextWindow }
                new_keyfile.insert(new_keyfile.size() - 1, " (workspace=[current]) ");
                mark_line_change = true;
                state = SCAN;
            }
            break;
       case CLOSE_STATIC_GROUP:
            if (c == '}') {
                new_keyfile += " (workspace=[current]) ";
                mark_line_change = true;
                state = SCAN;
            }
            break;
        };

        if (*pos == '\n') { // a new line or a comment resets the state
            if (mark_line_change)
                new_keyfile.insert(new_keyfile.size() - 1, " !! FBCV13 !!");
            mark_line_change = false;
            last_word.clear();
            state = SCAN;
        } else if (*pos == '#' || *pos == '!') {
            last_word.clear();
            state = COMMENT;
        }

    }

    new_keyfile += "\n"; // just for good looks
    write_file(keyfilename, new_keyfile);
}

typedef FbTk::VectorTraits<FbTk::StringTraits> StringVectorTraits;
typedef FbTk::Resource<std::vector<std::string>, StringVectorTraits> StringVectorResource;

struct ScreenResource {
    typedef FbTk::VectorTraits<FbTk::EnumTraits<WinButtonType> > WinButtonsTraits;
    static WinButtonType titlebar_left_[];
    static WinButtonType titlebar_right_[];

    ScreenResource(FbTk::ResourceManager_base &rm,
            const std::string &name, const std::string &altname);

    StringVectorResource workspace_names;

    FbTk::BoolResource opaque_move, full_max,
        max_ignore_inc, max_disable_move, max_disable_resize,
        workspace_warping, show_window_pos, auto_raise, click_raises;
    FbTk::StringResource default_deco;
    PlacementResource tab_placement;
    FbTk::StringResource windowmenufile;
    FbTk::UIntResource typing_delay;
    FbTk::IntResource workspaces, edge_snap_threshold, focused_alpha,
        unfocused_alpha, menu_alpha;
    FbTk::RangedIntResource menu_delay;
    FbTk::IntResource  tab_width, tooltip_delay;
    FbTk::BoolResource allow_remote_actions;
    FbTk::BoolResource clientmenu_use_pixmap;
//    FbTk::BoolResource tabs_use_pixmap;
    FbTk::BoolResource max_over_tabs;
    FbTk::BoolResource default_internal_tabs;

    FbTk::IntResource demands_attention_timeout;
    FbTk::StringResource timeformat;
    FbTk::Resource<FocusModel, FbTk::EnumTraits<FocusModel> > focus_model;
    FbTk::Resource<TabFocusModel, FbTk::EnumTraits<TabFocusModel> > tab_focus_model;
    FbTk::BoolResource focus_new;
//    FbTk::StringResource mode;
    FbTk::Resource<
        FbTk::Container::Alignment, FbTk::EnumTraits<FbTk::Container::Alignment>
    > alignment;
    FbTk::IntResource client_width;
    FbTk::UIntResource client_padding;
    FbTk::BoolResource use_pixmap;
    FbTk::Resource<RowDirection, FbTk::EnumTraits<RowDirection> > row_direction;
    FbTk::Resource<ColumnDirection, FbTk::EnumTraits<ColumnDirection> > col_direction;
    FbTk::Resource<PlacementPolicy, FbTk::EnumTraits<PlacementPolicy> > placement_policy;
    FbTk::BoolResource kde_dockapp, slit_auto_hide, slit_maximize_over;
//    PlacementResource slit_placement;
    FbTk::IntResource slit_alpha, slit_on_head;
    FbTk::Resource<LayerType, FbTk::EnumTraits<LayerType> > slit_layernum;
    FbTk::BoolResource toolbar_auto_hide, toolbar_maximize_over, visible;
    FbTk::IntResource width_percent;
    FbTk::IntResource toolbar_alpha;
    FbTk::Resource<LayerType, FbTk::EnumTraits<LayerType> > toolbar_layernum;
    FbTk::IntResource toolbar_on_head;
    PlacementResource toolbar_placement;
    FbTk::IntResource height;
    FbTk::StringResource tools;
    FbTk::Resource<std::vector<WinButtonType>, WinButtonsTraits> titlebar_left, titlebar_right;
};

WinButtonType ScreenResource::titlebar_left_[] = { STICKBUTTON };
WinButtonType ScreenResource::titlebar_right_[] = {
    MINIMIZEBUTTON,
    MAXIMIZEBUTTON,
    CLOSEBUTTON
};

ScreenResource::ScreenResource(FbTk::ResourceManager_base &rm,
                    const std::string &name, const std::string &altname) :
    workspace_names(rm, std::vector<std::string>(), name + ".workspaceNames",
            altname + ".WorkspaceNames", StringVectorTraits(",") ),
    opaque_move(rm, true, name + ".opaqueMove", altname + ".OpaqueMove"),
    full_max(rm, false, name + ".fullMaximization", altname + ".FullMaximization"),
    max_ignore_inc(rm, true, name + ".maxIgnoreIncrement", altname + ".MaxIgnoreIncrement"),
    max_disable_move(rm, false, name + ".maxDisableMove", altname + ".MaxDisableMove"),
    max_disable_resize(rm, false, name + ".maxDisableResize", altname + ".MaxDisableResize"),
    workspace_warping(rm, true, name + ".workspacewarping", altname + ".WorkspaceWarping"),
    show_window_pos(rm, false, name + ".showwindowposition", altname + ".ShowWindowPosition"),
    auto_raise(rm, true, name + ".autoRaise", altname + ".AutoRaise"),
    click_raises(rm, true, name + ".clickRaises", altname + ".ClickRaises"),
    default_deco(rm, "NORMAL", name + ".defaultDeco", altname + ".DefaultDeco"),
    tab_placement(rm, TOPLEFT, name + ".tab.placement", altname + ".Tab.Placement"),
    windowmenufile(rm, "~/.fluxbox/windowmenu", name + ".windowMenu", altname + ".WindowMenu"),
    typing_delay(rm, 0, name + ".noFocusWhileTypingDelay", altname + ".NoFocusWhileTypingDelay"),
    workspaces(rm, 4, name + ".workspaces", altname + ".Workspaces"),
    edge_snap_threshold(rm, 10, name + ".edgeSnapThreshold", altname + ".EdgeSnapThreshold"),
    focused_alpha(rm, 255, name + ".window.focus.alpha", altname + ".Window.Focus.Alpha"),
    unfocused_alpha(rm, 255, name + ".window.unfocus.alpha", altname + ".Window.Unfocus.Alpha"),
    menu_alpha(rm, 255, name + ".menu.alpha", altname + ".Menu.Alpha"),
    menu_delay(rm, 200, name + ".menuDelay", altname + ".MenuDelay",
            FbTk::RangedIntTraits(0, 5000)),
    tab_width(rm, 64, name + ".tab.width", altname + ".Tab.Width"),
    tooltip_delay(rm, 500, name + ".tooltipDelay", altname + ".TooltipDelay"),
    allow_remote_actions(rm, false,
            name + ".allowRemoteActions", altname + ".AllowRemoteActions"),
    clientmenu_use_pixmap(rm, true,
            name + ".clientMenu.usePixmap", altname + ".ClientMenu.UsePixmap"),
//  declared in main()
//  tabs_use_pixmap(rm, true, name + ".tabs.usePixmap", altname + ".Tabs.UsePixmap"),
    max_over_tabs(rm, false, name + ".tabs.maxOver", altname + ".Tabs.MaxOver"),
    default_internal_tabs(rm, true, name + ".tabs.intitlebar", altname + ".Tabs.InTitlebar"),

    demands_attention_timeout(rm, 500, name + ".demandsAttentionTimeout",
            altname + ".DemandsAttentionTimeout"),
    timeformat(rm, std::string("%k:%M"), name + ".strftimeFormat", altname + ".StrftimeFormat"),
    focus_model(rm, CLICKFOCUS, name + ".focusModel", altname + ".FocusModel"),
    tab_focus_model(rm, CLICKTABFOCUS,
            name + ".tabFocusModel", altname + ".TabFocusModel"),
    focus_new(rm, true, name + ".focusNewWindows", altname + ".FocusNewWindows"),
//  declared in main()
//  mode(rm, "{static groups} (workspace)", name + ".iconbar.mode", altname + ".Iconbar.Mode"),
    alignment(rm, FbTk::Container::RELATIVE,
            name + ".iconbar.alignment", altname + ".Iconbar.Alignment"),
    client_width(rm, 128, name + ".iconbar.iconWidth", altname + ".Iconbar.IconWidth"),
    client_padding(rm, 10,
            name + ".iconbar.iconTextPadding", altname + ".Iconbar.IconTextPadding"),
    use_pixmap(rm, true, name + ".iconbar.usePixmap", altname + ".Iconbar.UsePixmap"),
    row_direction(rm, LEFTRIGHTDIRECTION,
            name + ".rowPlacementDirection", altname + ".RowPlacementDirection"),
    col_direction(rm, TOPBOTTOMDIRECTION,
            name + ".colPlacementDirection", altname + ".ColPlacementDirection"),
    placement_policy(rm, ROWMINOVERLAPPLACEMENT,
            name + ".windowPlacement", altname + ".WindowPlacement"),
    kde_dockapp(rm, true, name + ".slit.acceptKdeDockapps", altname + ".Slit.AcceptKdeDockapps"),
    slit_auto_hide(rm, false, name + ".slit.autoHide", altname + ".Slit.AutoHide"),
    slit_maximize_over(rm, false, name + ".slit.maxOver", altname + ".Slit.MaxOver"),
//  declared in main()
//  slit_placement(rm, RIGHTBOTTOM, name + ".slit.placement", altname + ".Slit.Placement"),
    slit_alpha(rm, 255, name + ".slit.alpha", altname + ".Slit.Alpha"),
    slit_on_head(rm, 0, name + ".slit.onhead", altname + ".Slit.onHead"),
    slit_layernum(rm, LAYERDOCK, name + ".slit.layer", altname + ".Slit.Layer"),
    toolbar_auto_hide(rm, false, name + ".toolbar.autoHide", altname + ".Toolbar.AutoHide"),
    toolbar_maximize_over(rm, false, name + ".toolbar.maxOver", altname + ".Toolbar.MaxOver"),
    visible(rm, true, name + ".toolbar.visible", altname + ".Toolbar.Visible"),
    width_percent(rm, 100,
            name + ".toolbar.widthPercent", altname + ".Toolbar.WidthPercent"),
    toolbar_alpha(rm, 255,
            name + ".toolbar.alpha", altname + ".Toolbar.Alpha"),
    toolbar_layernum(rm, LAYERDOCK, name + ".toolbar.layer", altname + ".Toolbar.Layer"),
    toolbar_on_head(rm, 1,
            name + ".toolbar.onhead", altname + ".Toolbar.onHead"),
    toolbar_placement(rm, BOTTOMCENTER,
            name + ".toolbar.placement", altname + ".Toolbar.Placement"),
    height(rm, 0, name + ".toolbar.height", altname + ".Toolbar.Height"),
    tools(rm, "prevworkspace, workspacename, nextworkspace, iconbar, systemtray, clock",
            name + ".toolbar.tools", altname + ".Toolbar.Tools"),
    titlebar_left(rm, std::vector<WinButtonType>(titlebar_left_, titlebar_left_+1),
            name + ".titlebar.left", altname + ".Titlebar.Left", WinButtonsTraits(" \t\n")),
    titlebar_right(rm, std::vector<WinButtonType>(titlebar_right_, titlebar_right_+3),
            name + ".titlebar.right", altname + ".Titlebar.Right", WinButtonsTraits(" \t\n"))
{}


void update_lua_resource_manager(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {
    if( dynamic_cast<FbTk::LResourceManager *>(rm.get()) ) {
        // there's nothing to do, we already have a lua resource manager
        // this shouldn't happen, since all lua init files should have versions >= 14
        return;
    }

    FbTk::BoolResource rc_ignoreborder(*rm, false, "ignoreBorder", "IgnoreBorder");
    FbTk::BoolResource rc_pseudotrans(*rm, false,
            "forcePseudoTransparency", "forcePseudoTransparency");
    FbTk::IntResource rc_colors_per_channel(*rm, 4, "colorsPerChannel", "ColorsPerChannel");
    FbTk::IntResource rc_double_click_interval(*rm, 250,
            "doubleClickInterval", "DoubleClickInterval");
    FbTk::IntResource rc_tabs_padding(*rm, 0, "tabPadding", "TabPadding");
    FbTk::StringResource rc_stylefile(*rm, DEFAULTSTYLE, "styleFile", "StyleFile");
    FbTk::StringResource rc_styleoverlayfile(*rm, "~/.fluxbox/overlay",
            "styleOverlay", "StyleOverlay");
    FbTk::StringResource rc_menufile(*rm, "~/.fluxbox/menu", "menuFile", "MenuFile");
    FbTk::StringResource rc_slitlistfile(*rm, "~/.fluxbox/slitlist",
            "slitlistFile", "SlitlistFile");

/*  Key and apps file resources are declared in run_updates
    FbTk::StringResource rc_keyfile(*rm, "~/.fluxbox/keys", "keyFile", "KeyFile");
    FbTk::StringResource rc_appsfile(*rm, "~/.fluxbox/apps", "appsFile", "AppsFile");*/

    FbTk::Resource<
        TabsAttachArea, FbTk::EnumTraits<TabsAttachArea>
    > rc_tabs_attach_area(*rm, ATTACH_AREA_WINDOW, "tabsAttachArea", "TabsAttachArea");
    FbTk::UIntResource rc_cache_life(*rm, 5, "cacheLife", "CacheLife");
    FbTk::UIntResource rc_cache_max(*rm, 200, "cacheMax", "CacheMax");
    FbTk::Resource<time_t, FbTk::IntTraits<time_t> > rc_auto_raise_delay(*rm, 250, "autoRaiseDelay", "AutoRaiseDelay");

    ScreenResource screen_resource(*rm, "screen0", "Screen0");

    rm.reset(new FbTk::LResourceManager(dynamic_cast<FbTk::ResourceManager &>(*rm), l));
}

void
update_move_slitlist_to_init_file(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {
    FbTk::StringResource rc_slitlistfile(*rm, "~/.fluxbox/slitlist",
            "slitlistFile");
    StringVectorResource rc_slitlist(*rm, std::vector<std::string>(), "screen0.slit.clientList",
            StringVectorTraits(","));

    std::istringstream f(read_file(FbTk::StringUtil::expandFilename(*rc_slitlistfile)));
    std::string line;
    while(getline(f, line))
        rc_slitlist->push_back(line);
}

void update_keys_for_lua(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l) {
    extern const char update_keys[];
    extern const unsigned int update_keys_size;

    l.checkstack(2);
    lua::stack_sentry s(l);

    FbTk::StringResource &rc_keyfile = rm->getResource<string, FbTk::StringTraits>("keyFile");

    l.loadstring(update_keys, update_keys_size);
    l.pushstring(read_file(FbTk::StringUtil::expandFilename(*rc_keyfile)));
    l.call(1, 1);
    *rc_keyfile = string(*rc_keyfile) + ".lua";
    write_file(FbTk::StringUtil::expandFilename(*rc_keyfile), l.tostring(-1));
}

/*------------------------------------------------------------------*\
\*------------------------------------------------------------------*/

struct Update {
    int version;
    void (*update)(std::auto_ptr<FbTk::ResourceManager_base>& rm, FbTk::Lua &l);
};

const Update UPDATES[] = {
    {  1, update_add_mouse_evens_to_keys },
    {  2, update_move_groups_entries_to_apps_file },
    {  3, update_move_toolbar_wheeling_to_keys_file },
    {  4, update_move_modkey_to_keys_file },
    {  5, update_window_patterns_for_iconbar },
    {  6, update_move_titlebar_actions_to_keys_file },
    {  7, update_added_starttabbing_command },
    {  8, update_disable_icons_in_tabs_for_backwards_compatibility },
    {  9, update_change_format_of_split_placement_menu },
    { 10, update_update_keys_file_for_nextwindow_syntax_changes },
    { 11, update_keys_for_ongrip_onwindowborder },
    { 12, update_keys_for_activetab },
    { 13, update_limit_nextwindow_to_current_workspace },
    { 14, update_lua_resource_manager },
    { 15, update_move_slitlist_to_init_file },
    { 16, update_keys_for_lua }
};

/*------------------------------------------------------------------*\
\*------------------------------------------------------------------*/

int run_updates(int old_version, std::auto_ptr<FbTk::ResourceManager_base> &rm, FbTk::Lua &l) {
    int new_version = old_version;

    FbTk::StringResource rc_keyfile(*rm, "~/.fluxbox/keys",
            "keyFile", "KeyFile");
    FbTk::StringResource rc_appsfile(*rm, "~/.fluxbox/apps",
            "appsFile", "AppsFile");

    for (size_t i = 0; i < sizeof(UPDATES) / sizeof(Update); ++i) {
        if (old_version < UPDATES[i].version) {
            UPDATES[i].update(rm, l);
            new_version = UPDATES[i].version;
        }
    }

    return new_version;
}

std::auto_ptr<FbTk::ResourceManager_base> try_load(const std::string &filename, FbTk::Lua &l) {
    _FB_USES_NLS;
    std::auto_ptr<FbTk::ResourceManager_base> r;
    try {
        r.reset(new FbTk::LResourceManager("session", l));
        r->doLoad(filename);
    }
    catch(std::runtime_error &) {
        try {
            r.reset(new FbTk::ResourceManager("session", "Session", filename.c_str(), false));
            r->doLoad(filename);
        }
        catch(std::runtime_error &) {
            r.reset();
        }
    }
    if(r.get()) {
        cerr << _FB_CONSOLETEXT(Update, Loading, "Loading resources from: ", "filename follows")
             << filename << endl;
    }
    return r;
}

int main(int argc, char **argv) {
    string rc_filename;
    string oldrc_filename;
    set<string> style_filenames;
    int i = 1;
    bool check = 0;
    pid_t fb_pid = 0;

    FbTk::NLSInit("fluxbox.cat");
    _FB_USES_NLS;

    for (; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-rc") {
            // look for alternative rc file to use

            if ((++i) >= argc) {
                cerr<<_FB_CONSOLETEXT(main, RCRequiresArg,
                              "error: '-rc' requires an argument", "the -rc option requires a file argument")<<endl;
                exit(1);
            }

            rc_filename = argv[i];
        } else if (arg == "-oldrc") {
            if ((++i) >= argc) {
                cerr<<_FB_CONSOLETEXT(main, RCRequiresArg,
                              "error: '-oldrc' requires an argument", "the -oldrc option requires a file argument")<<endl;
                return 1;
            }

            oldrc_filename = argv[i];
        } else if (arg == "-pid") {
            if ((++i) >= argc) {
                // need translations for this, too
                cerr<<"the -pid option requires a numeric argument"<<endl;
            } else
                fb_pid = atoi(argv[i]);
        } else if (arg == "-check") {
            check = true;
        } else if (arg == "-help" || arg == "-h") {
            // no NLS translations yet -- we'll just have to use English for now
            cout << "  -rc <string>\t\t\twhere to save the new resource file.\n"
                 << "  -oldrc <string>\t\tfile from which to load old resources (default = same as -rc).\n"
                 << "  -pid <int>\t\t\ttell fluxbox to reload configuration.\n"
                 << "  -check\t\t\tcheck version of this tool and the fluxbox config.\n"
                 << "  -help\t\t\t\tdisplay this help text and exit.\n\n"
                 << endl;
            exit(0);
        }
    }

    string filenames[4];
    if(!oldrc_filename.empty())
        filenames[0] = oldrc_filename;
    else if(!rc_filename.empty())
        filenames[0] = rc_filename;
    else {
        filenames[0] = getenv("HOME") + string("/.fluxbox/init.lua");
        filenames[1] = getenv("HOME") + string("/.fluxbox/init");
        filenames[2] = DEFAULT_INITFILE;
        filenames[3] = string(DEFAULT_INITFILE, string(DEFAULT_INITFILE).rfind(".lua"));
    }

    if (rc_filename.empty())
        rc_filename = getenv("HOME") + string("/.fluxbox/init.lua");

    FbTk::Lua l;
    std::auto_ptr<FbTk::ResourceManager_base> resource_manager;
    for(size_t i = 0; i < sizeof filenames / sizeof filenames[0]; ++i) {
        if(!filenames[i].empty()) {
            resource_manager = try_load(filenames[i], l);
            if(resource_manager.get()) {
                oldrc_filename = filenames[i];
                break;
            }
        }
    }
    if(!resource_manager.get()) {
        // This should only happen if system-wide init file is broken.
        // this is a fatal error for us
        return 1;
    }

    // run updates here
    // I feel like putting this in a separate function for no apparent reason

    FbTk::IntResource config_version(*resource_manager, 0,
            "configVersion", "ConfigVersion");

    if (check) {
        cout << oldrc_filename << ": " << *config_version << endl
            << "fluxbox-update_configs: " << UPDATES[sizeof(UPDATES)/sizeof(Update) - 1].version << endl;
        return 0;
    }

    FbTk::StringResource rc_iconbar_mode(*resource_manager, "{static groups} (workspace)",
            "screen0.iconbar.mode", "Screen0.Iconbar.Mode");
    FbTk::BoolResource rc_tabs_usepixmap(*resource_manager, true,
            "screen0.tabs.usePixmap", "Screen0.Tabs.UsePixmap");
    PlacementResource rc_slit_placement(*resource_manager, RIGHTBOTTOM,
            "screen0.slit.placement", "Screen0.Slit.Placement");
    FbTk::StringResource rc_slit_direction(*resource_manager, "Vertical",
            "screen0.slit.direction", "Screen0.Slit.Direction");

    int old_version = *config_version;
    int new_version = run_updates(old_version, resource_manager, l);
    if (new_version > old_version) {
        // configs were updated -- let's save our changes
        config_version = new_version;
        resource_manager->save(rc_filename.c_str());
        save_all_files();

#if defined(HAVE_SIGNAL_H) && !defined(_WIN32)
        // if we were given a fluxbox pid, send it a reconfigure signal
        if (fb_pid > 0)
            kill(fb_pid, SIGUSR2);
#endif // defined(HAVE_SIGNAL_H) && !defined(_WIN32)

    }

    return 0;
}

namespace {

set<string> modified_files;
// we may want to put a size limit on this cache, so it doesn't grow too big
map<string,string> file_cache;

}

// returns the contents of the file given, either from the cache or by reading
// the file from disk
string read_file(const string& filename) {
    // check if we already have the file in memory
    map<string,string>::iterator it = file_cache.find(filename);
    if (it != file_cache.end())
        return it->second;

    if (!FbTk::FileUtil::isRegularFile(filename.c_str())) {
        return "";
    }

    // nope, we'll have to read the file
    ifstream infile(filename.c_str());
    string whole_file = "";

    if (!infile) // failed to open file
        return whole_file;

    string linebuffer;
    while (!infile.eof()) {
        getline(infile, linebuffer);
        whole_file += linebuffer + "\n";
    }
    infile.close();

    file_cache[filename] = whole_file;
    return whole_file;
}

#ifdef NOT_USED
// remove the file from the cache, writing to disk if it's been changed
void forget_file(const string& filename) {
    map<string,string>::iterator cache_it = file_cache.find(filename);
    // check if we knew about the file to begin with
    if (cache_it == file_cache.end())
        return;

    // check if we've actually modified it
    set<string>::iterator mod_it = modified_files.find(filename);
    if (mod_it == modified_files.end()) {
        file_cache.erase(cache_it);
        return;
    }

    // flush our changes to disk and remove all traces
    ofstream outfile(filename.c_str());
    outfile << cache_it->second;
    file_cache.erase(cache_it);
    modified_files.erase(mod_it);
}
#endif // NOT_USED

// updates the file contents in the cache and marks the file as modified so it
// gets saved later
void write_file(const string& filename, const string &contents) {
    modified_files.insert(filename);
    file_cache[filename] = contents;
}

// actually save all the files we've modified
void save_all_files() {
    set<string>::iterator it = modified_files.begin();
    set<string>::iterator it_end = modified_files.end();
    for (; it != it_end; ++it) {
        ofstream outfile(it->c_str());
        outfile << file_cache[it->c_str()];
    }
    modified_files.clear();
}
