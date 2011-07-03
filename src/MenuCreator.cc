// MenuCreator.cc for Fluxbox
// Copyright (c) 2004-2008 Henrik Kinnunen (fluxgen at fluxbox dot org)
//                and Simon Bowden    (rathnor at users.sourceforge.net)
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
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "MenuCreator.hh"

#include "defaults.hh"
#include "Screen.hh"
#include "FbTk/CommandParser.hh"
#include "fluxbox.hh"
#include "Window.hh"
#include "WindowCmd.hh"
#include "CurrentWindowCmd.hh"
#include "WindowMenuAccessor.hh"

#include "ClientMenu.hh"
#include "WorkspaceMenu.hh"
#include "LayerMenu.hh"
#include "SendToMenu.hh"
#include "AlphaMenu.hh"
#include "Layer.hh"

#include "FbMenuParser.hh"
#include "StyleMenuItem.hh"
#include "RootCmdMenuItem.hh"

#include "FbTk/I18n.hh"
#include "FbTk/MultiButtonMenuItem.hh"
#include "FbTk/BoolMenuItem.hh"
#include "FbTk/RefCount.hh"
#include "FbTk/MacroCommand.hh"
#include "FbTk/SimpleCommand.hh"
#include "FbTk/StringUtil.hh"
#include "FbTk/FileUtil.hh"
#include "FbTk/MenuSeparator.hh"
#include "FbTk/Transparent.hh"

#include <iostream>
#include <algorithm>

using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::list;
using std::less;
using FbTk::AutoReloadHelper;

namespace {

typedef FbTk::RefCount<FbTk::Menu> RefMenu;
typedef FbTk::RefCount<FbTk::Command<void> > RefCmd;

RefMenu createStyleMenu(int screen_number, const string &label,
                     AutoReloadHelper *reloader, const string &directory) {

    FbTk::RefCount<FbMenu> menu(MenuCreator::createMenu(label, screen_number));

    // perform shell style ~ home directory expansion
    string stylesdir(FbTk::StringUtil::expandFilename(directory));

    if (!FbTk::FileUtil::isDirectory(stylesdir.c_str()))
        return menu;

    if (reloader)
        reloader->addFile(stylesdir);

    FbTk::Directory dir(stylesdir.c_str());

    // create a vector of all the filenames in the directory
    // add sort it
    vector<string> filelist(dir.entries());
    for (size_t file_index = 0; file_index < dir.entries(); ++file_index)
        filelist[file_index] = dir.readFilename();

    sort(filelist.begin(), filelist.end(), less<string>());

    // for each file in directory add filename and path to menu
    for (size_t file_index = 0; file_index < dir.entries(); file_index++) {
        string style(stylesdir + '/' + filelist[file_index]);
        // add to menu only if the file is a regular file, and not a
        // .file or a backup~ file
        if ((FbTk::FileUtil::isRegularFile(style.c_str()) &&
             (filelist[file_index][0] != '.') &&
             (style[style.length() - 1] != '~')) ||
            FbTk::FileUtil::isRegularFile((style + "/theme.cfg").c_str()) ||
            FbTk::FileUtil::isRegularFile((style + "/style.cfg").c_str()))
            menu->insert(new StyleMenuItem(filelist[file_index], style));
    }
    // update menu graphics
    menu->updateMenu();
    return menu;
}

RefMenu createRootCmdMenu(int screen_number, const string &label,
                       const string &directory, AutoReloadHelper *reloader,
                       const string &cmd) {

    FbTk::RefCount<FbMenu> menu(MenuCreator::createMenu(label, screen_number));

    // perform shell style ~ home directory expansion
    string rootcmddir(FbTk::StringUtil::expandFilename(directory));

    if (!FbTk::FileUtil::isDirectory(rootcmddir.c_str()))
        return menu;

    if (reloader)
        reloader->addFile(rootcmddir);

    FbTk::Directory dir(rootcmddir.c_str());

    // create a vector of all the filenames in the directory
    // add sort it
    vector<string> filelist(dir.entries());
    for (size_t file_index = 0; file_index < dir.entries(); ++file_index)
        filelist[file_index] = dir.readFilename();

    sort(filelist.begin(), filelist.end(), less<string>());

    // for each file in directory add filename and path to menu
    for (size_t file_index = 0; file_index < dir.entries(); file_index++) {

        string rootcmd(rootcmddir+ '/' + filelist[file_index]);
        // add to menu only if the file is a regular file, and not a
        // .file or a backup~ file
        if ((FbTk::FileUtil::isRegularFile(rootcmd.c_str()) &&
             (filelist[file_index][0] != '.') &&
             (rootcmd[rootcmd.length() - 1] != '~')))
            menu->insert(new RootCmdMenuItem(filelist[file_index], rootcmd, cmd));
    }
    // update menu graphics
    menu->updateMenu();
    return menu;
}


class ParseItem {
public:
    explicit ParseItem(FbTk::Menu *menu):m_menu(menu) {}

    void load(FbTk::Parser &p, FbTk::StringConvertor &m_labelconvertor) {
        p>>m_key>>m_label>>m_cmd>>m_icon;
        m_label.second = m_labelconvertor.recode(m_label.second);
    }
    const string &icon() const { return m_icon.second; }
    const string &command() const { return m_cmd.second; }
    const string &label() const { return m_label.second; }
    const string &key() const { return m_key.second; }
    FbTk::Menu *menu() { return m_menu; }
private:
    FbTk::Parser::Item m_key, m_label, m_cmd, m_icon;
    FbTk::Menu *m_menu;
};

class MenuContext: public LayerObject {
public:
    void moveToLayer(int layer_number) {
        if (FbMenu::window() == 0)
            return;
        FbMenu::window()->moveToLayer(layer_number);
    }
    int layerNumber() const {
        if (FbMenu::window() == 0)
            return -1;
        return FbMenu::window()->layerItem().getLayerNum();
    }

};

void parseMenu(FbTk::Parser &pars, FbTk::Menu &menu,
               FbTk::StringConvertor &label_convertor,
               AutoReloadHelper *reloader) {
    ParseItem pitem(&menu);
    while (!pars.eof()) {
        pitem.load(pars, label_convertor);
        if (pitem.key() == "end")
            return;
//        translateMenuItem(pars, pitem, label_convertor, reloader);
    }
}

bool getStart(FbMenuParser &parser, string &label, FbTk::StringConvertor &labelconvertor) {
    ParseItem pitem(0);
    while (!parser.eof()) {
        // get first begin line
        pitem.load(parser, labelconvertor);
        if (pitem.key() == "begin") {
            break;
        }
    }
    if (parser.eof())
        return false;

    label = pitem.label();
    return true;
}

string getField(lua::state &l, int pos, const char *field, FbTk::StringConvertor *conv = NULL) {
    lua::stack_sentry s(l);
    l.checkstack(1);
    _FB_USES_NLS;

    string val;
    l.rawgetfield(pos, field); {
        if(l.isstring(-1))
            val = l.tostring(-1);
        else if(! l.isnil(-1)) {
            fprintf(stderr, _FB_CONSOLETEXT(Menu, FieldNotString,
                        "Warning: Menu field %s is not a string", "One %s for field name.").c_str(),
                    field);
            fputs("\n", stderr);
        }
    } l.pop();
    return conv ? conv->recode(val) : val;
}

void
createMenu_(FbTk::Menu &inject_into, lua::state &l, FbTk::StringConvertor &conv,
            FbTk::AutoReloadHelper *reloader);

void
insertMenuItem(lua::state &l, FbTk::Menu &menu, FbTk::StringConvertor &parent_conv,
                FbTk::AutoReloadHelper *reloader) {
    lua::stack_sentry s(l, -1);
    l.checkstack(1);

    if(l.type(-1) != lua::TTABLE)
        throw std::runtime_error(_FB_CONSOLETEXT(Menu, MenuNotTable, "Warning: Menu is not a lua table", "Menu is not a lua table"));

    // if menu specifies an encoding, create a convertor for it
    std::auto_ptr<FbTk::StringConvertor> my_conv;

    FbTk::StringConvertor *conv = &parent_conv;
    const string &encoding = getField(l, -1, "encoding");
    if(! encoding.empty()) {
        my_conv.reset(new FbTk::StringConvertor(FbTk::StringConvertor::ToFbString));
        conv = my_conv.get();
        conv->setSource(encoding);
    }

    const string &str_label = getField(l, -1, "label", conv);
    const string &str_key = getField(l, -1, "type");
    const FbTk::CommandParser<void> &parser = FbTk::CommandParser<void>::instance();
    int screen_number = menu.screenNumber();
    BScreen *screen = Fluxbox::instance()->findScreen(screen_number);
    size_t old_size = menu.numberOfItems();

    // first items that don't need additional parameters
    if(str_key == "separator")
        menu.insert(new FbTk::MenuSeparator());
    else if(str_key == "nop") {
        int size = menu.insert(str_label);
        menu.setItemEnabled(size-1, false);
    } else if(str_key == "icons") {
        RefMenu submenu = MenuCreator::createMenuType("iconmenu", screen_number);
        if (! submenu)
            return;
        if (str_label.empty())
            menu.insert(_FB_XTEXT(Menu, Icons, "Icons", "Iconic windows menu title"), submenu);
        else
            menu.insert(str_label, submenu);
    } else if (str_key == "exit") { // exit
        RefCmd exit_cmd(FbTk::CommandParser<void>::instance().parse("exit"));
        if (str_label.empty())
            menu.insert(_FB_XTEXT(Menu, Exit, "Exit", "Exit Command"), exit_cmd);
        else
            menu.insert(str_label, exit_cmd);
    } else if (str_key == "config") {
        menu.insert(str_label, RefMenu(screen->configMenu()) );
    } else if(str_key == "menu") {
        RefMenu t(MenuCreator::createMenu("", screen_number));
        l.pushvalue(-1);
        createMenu_(*t, l, *conv, reloader);
        menu.insert(str_label, t);
    } else {
        // items that have a parameter
        const string &str_cmd = getField(l, -1, "param");

        if(str_key == "command") {
            menu.insert(str_label, RefCmd( parser.parse(str_cmd)) );
        } else if(str_key == "exec") {
            menu.insert(str_label, RefCmd( parser.parse("exec", str_cmd)) );
        } else if(str_key == "style")
            menu.insert(new StyleMenuItem(str_label, str_cmd));
        else if (str_key == "stylesdir")
            menu.insert(str_label,
                    createStyleMenu(screen_number, str_label, reloader, str_cmd));
        else if (str_key == "wallpapers") {
            string program = getField(l, -1, "program");
            if(program.empty())
                program = realProgramName("fbsetbg");
            menu.insert(str_label, createRootCmdMenu(screen_number, str_label, str_cmd,
                                                        reloader, program) );
        } else if (str_key == "workspaces") {
            menu.insert(str_label, RefMenu(screen->workspaceMenu()) );
        } else {
            // finally, try window-related commands
            MenuCreator::createWindowMenuItem(str_key, str_label, menu);
        }
    }

    const string &icon = getField(l, -1, "icon");
    if(! icon.empty()) {
        while(old_size < menu.numberOfItems())
            menu.find(old_size++)->setIcon(icon, menu.screenNumber());
    }
}

void
createMenu_(FbTk::Menu &inject_into, lua::state &l, FbTk::StringConvertor &conv,
            FbTk::AutoReloadHelper *reloader) {

    lua::stack_sentry s(l, -1);
    l.checkstack(1);

    inject_into.setLabel(getField(l, -1, "label", &conv));

    for(int i = 1; l.rawgeti(-1, i), !l.isnil(-1); ++i) {
        try {
            insertMenuItem(l, inject_into, conv, reloader);
        }
        catch(std::runtime_error &e) {
            cerr << e.what() << endl;
        }
    } l.pop();

    l.pop();
}

} // end of anonymous namespace

void
MenuCreator::createMenu(FbTk::Menu &inject_into, lua::state &l, FbTk::AutoReloadHelper *reloader) {
    lua::stack_sentry s(l, -1);

    if(l.type(-1) != lua::TTABLE) {
        cerr << _FB_CONSOLETEXT(Menu, MenuNotTable, "Warning: Menu is not a lua table",
                        "Menu is not a lua table") << endl;
        return;
    }

    std::auto_ptr<FbTk::StringConvertor> conv(new FbTk::StringConvertor(FbTk::StringConvertor::ToFbString));

    // if menu specifies an encoding, create a convertor for it
    const std::string &enc = getField(l, -1, "encoding");
    if(!enc.empty())
        conv->setSource(enc);

    createMenu_(inject_into, l, *conv, reloader);
}


FbMenu *MenuCreator::createMenu(const string &label, int screen_number) {
    BScreen *screen = Fluxbox::instance()->findScreen(screen_number);
    if (screen == 0)
        return 0;

    FbMenu *menu = new FbMenu(screen->menuTheme(),
                                  screen->imageControl(),
                                  *screen->layerManager().getLayer(ResourceLayer::MENU));
    if (!label.empty())
        menu->setLabel(label);

    return menu;
}

void MenuCreator::createFromFile(const string &filename,
                                 FbTk::Menu &inject_into,
                                 AutoReloadHelper *reloader) {
    string real_filename = FbTk::StringUtil::expandFilename(filename);

    lua::state &l = Fluxbox::instance()->lua();
    l.checkstack(1);
    lua::stack_sentry s(l);

    l.loadfile(real_filename.c_str());
    l.call(0, 1);
    createMenu(inject_into, l, reloader);
}

FbTk::RefCount<FbMenu> MenuCreator::createMenuType(const string &type, int screen_num) {
    FbTk::RefCount<FbMenu> menu;
    BScreen *screen = Fluxbox::instance()->findScreen(screen_num);

    if (type == "iconmenu")
        menu.reset(new ClientMenu(*screen, screen->iconList(),
                              true)); // listen to icon list changes
    else if (type == "workspacemenu")
         menu.reset(new WorkspaceMenu(*screen));

    return menu;
}

bool MenuCreator::createWindowMenuItem(const string &type,
                                       const string &label,
                                       FbTk::Menu &menu) {
    _FB_USES_NLS;

    static MenuContext context;

    if (type == "shade") {
        static WindowMenuAccessor<bool> res(&FluxboxWindow::isShaded, &FluxboxWindow::setShaded, false);
        menu.insert(new FbTk::BoolMenuItem(
                        label.empty()?_FB_XTEXT(Windowmenu, Shade, "Shade", "Shade the window"):label, 
                        res));

    } else if (type == "maximize") {
        RefCmd maximize_cmd(new WindowCmd<void>(&FluxboxWindow::maximizeFull));
        RefCmd maximize_vert_cmd(new WindowCmd<void>(&FluxboxWindow::maximizeVertical));
        RefCmd maximize_horiz_cmd(new WindowCmd<void>(&FluxboxWindow::maximizeHorizontal));
        FbTk::MultiButtonMenuItem *maximize_item =
            new FbTk::MultiButtonMenuItem(3,
                                          label.empty()?
                                          _FB_XTEXT(Windowmenu, Maximize,
                                                  "Maximize", "Maximize the window"):
                                          label);
        // create maximize item with:
        // button1: Maximize normal
        // button2: Maximize Vertical
        // button3: Maximize Horizontal
        maximize_item->setCommand(1, maximize_cmd);
        maximize_item->setCommand(2, maximize_vert_cmd);
        maximize_item->setCommand(3, maximize_horiz_cmd);
        menu.insert(maximize_item);
    } else if (type == "iconify") {
        static WindowMenuAccessor<bool> res(&FluxboxWindow::isIconic, &FluxboxWindow::setIconic, false);
        menu.insert(new FbTk::BoolMenuItem(
                        label.empty() ?
                        _FB_XTEXT(Windowmenu, Iconify,
                                  "Iconify", "Iconify the window") :
                        label, res));
    } else if (type == "close") {
        RefCmd close_cmd(new WindowCmd<void>(&FluxboxWindow::close));
        menu.insert(label.empty() ?
                    _FB_XTEXT(Windowmenu, Close,
                              "Close", "Close the window") :
                    label, close_cmd);
    } else if (type == "kill" || type == "killwindow") {
        RefCmd kill_cmd(new WindowCmd<void>(&FluxboxWindow::kill));
        menu.insert(label.empty() ?
                    _FB_XTEXT(Windowmenu, Kill,
                              "Kill", "Kill the window"):
                    label, kill_cmd);
    } else if (type == "lower") {
        RefCmd lower_cmd(new WindowCmd<void>(&FluxboxWindow::lower));
        menu.insert( label.empty() ?
                     _FB_XTEXT(Windowmenu, Lower,
                               "Lower", "Lower the window"):
                     label, lower_cmd);
    } else if (type == "raise") {
        RefCmd raise_cmd(new WindowCmd<void>(&FluxboxWindow::raise));
        menu.insert(label.empty() ?
                    _FB_XTEXT(Windowmenu, Raise,
                              "Raise", "Raise the window"):
                    label, raise_cmd);

    } else if (type == "stick") {
        static WindowMenuAccessor<bool> res(&FluxboxWindow::isStuck, &FluxboxWindow::setStuck, false);
        menu.insert(new FbTk::BoolMenuItem(
                        label.empty() ?
                        _FB_XTEXT(Windowmenu, Stick,
                                  "Stick", "Stick the window"):
                        label, res));
    } else if (type == "settitledialog") {
        RefCmd setname_cmd(new SetTitleDialogCmd());
        menu.insert(label.empty() ?
                    _FB_XTEXT(Windowmenu, SetTitle,
                              "Set Title", "Change the title of the window"):
                    label, setname_cmd);
#ifdef HAVE_XRENDER
    } else if (type == "alpha") {
        if (FbTk::Transparent::haveComposite() || 
            FbTk::Transparent::haveRender()) {
            BScreen *screen = Fluxbox::instance()->findScreen(menu.screenNumber());
            if (screen == 0)
                return false;

            RefMenu submenu( new AlphaMenu(screen->menuTheme(), screen->imageControl(),
                              *screen->layerManager().getLayer(ResourceLayer::MENU)) );
            submenu->disableTitle();
            menu.insert(label.empty() ? _FB_XTEXT(Configmenu, Transparency, "Transparency",
                                                  "Menu containing various transparency options"): label,
                        submenu);
        }
#endif // HAVE_XRENDER
    } else if (type == "extramenus") {
        BScreen *screen = Fluxbox::instance()->findScreen(menu.screenNumber());
        BScreen::ExtraMenus::iterator it = screen->extraWindowMenus().begin();
        BScreen::ExtraMenus::iterator it_end = screen->extraWindowMenus().end();
        for (; it != it_end; ++it) {
            it->second->disableTitle();
            menu.insert(it->first, it->second);
        }

    } else if (type == "sendto") {
        menu.insert(label.empty() ? _FB_XTEXT(Windowmenu, SendTo, "Send To...", "Send to menu item name"):
                    label, RefMenu(new SendToMenu(*Fluxbox::instance()->findScreen(menu.screenNumber()))) );
    } else if (type == "layer") {
        BScreen *screen = Fluxbox::instance()->findScreen(menu.screenNumber());
        if (screen == 0)
            return false;

        RefMenu submenu( new LayerMenu(screen->menuTheme(),
                                            screen->imageControl(),
                                            *screen->layerManager().getLayer(ResourceLayer::MENU),
                                            &context,
                                            false) );
        submenu->disableTitle();
        menu.insert(label.empty()?_FB_XTEXT(Windowmenu, Layer, "Layer ...", "Layer menu"):label, submenu);


    } else if (type == "separator") {
        menu.insert(new FbTk::MenuSeparator());
    } else
        return false;

    return true;
}
