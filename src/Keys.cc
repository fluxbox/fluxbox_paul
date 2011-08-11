// Keys.cc for Fluxbox - an X11 Window manager
// Copyright (c) 2001 - 2006 Henrik Kinnunen (fluxgen at fluxbox dot org)
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

#include "Keys.hh"

#include "FbCommands.hh"
#include "fluxbox.hh"
#include "Screen.hh"
#include "WinClient.hh"
#include "WindowCmd.hh"
#include "Debug.hh"

#include "FbTk/EventManager.hh"
#include "FbTk/StringUtil.hh"
#include "FbTk/FileUtil.hh"
#include "FbTk/App.hh"
#include "FbTk/Command.hh"
#include "FbTk/RefCount.hh"
#include "FbTk/KeyUtil.hh"
#include "FbTk/LuaUtil.hh"
#include "FbTk/CommandParser.hh"
#include "FbTk/I18n.hh"
#include "FbTk/AutoReloadHelper.hh"
#include "FbTk/STLUtil.hh"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H


#ifdef HAVE_CCTYPE
  #include <cctype>
#else
  #include <ctype.h>
#endif	// HAVE_CCTYPE

#ifdef HAVE_CSTDIO
  #include <cstdio>
#else
  #include <stdio.h>
#endif
#ifdef HAVE_CSTDLIB
  #include <cstdlib>
#else
  #include <stdlib.h>
#endif
#ifdef HAVE_CERRNO
  #include <cerrno>
#else
  #include <errno.h>
#endif
#ifdef HAVE_CSTRING
  #include <cstring>
#else
  #include <string.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif	// HAVE_SYS_TYPES_H

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif	// HAVE_SYS_WAIT_H

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif	// HAVE_UNISTD_H

#ifdef	HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif	// HAVE_SYS_STAT_H

#include <X11/Xproto.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <memory>

using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;
using std::pair;

namespace {

// candidate for FbTk::StringUtil ?
int extractKeyFromString(const std::string& in, const char* start_pattern, unsigned int& key) {

    int ret = 0;

    if (strstr(in.c_str(), start_pattern) != 0) {

        unsigned int tmp_key = 0;
        if (FbTk::StringUtil::extractNumber(in.substr(strlen(start_pattern)), tmp_key)) {

            key = tmp_key;
            ret = 1;
        }
    }

    return ret;
}

struct KeyError: public std::runtime_error {
    KeyError(const std::string &e) : std::runtime_error(e) {}
};

const char keymode_metatable[] = "Keys::keymode_metatable";
const char default_keymode[] = "default_keymode";

} // end of anonymous namespace

// helper class 'keytree'
class Keys::t_key: private FbTk::NotCopyable {
public:

    // typedefs
    typedef std::list<RefKey> keylist_t;
    typedef std::pair<keylist_t::iterator, t_key *> FindPair;

    static void initKeys(FbTk::Lua &l);
    static int addBinding(lua::state *l);
    static int newKeyMode(lua::state *l);
    static int index(lua::state *l);
    static int newindex(lua::state *l);
    static int clear(lua::state *l);

    bool equalExact(const RefKey &x) {
        return type == x->type && key == x->key && context == x->context
                    && isdouble == x->isdouble && mod == x->mod;
    }

    // constructor
    t_key(int type = 0, unsigned int mod = 0, unsigned int key = 0,
            const std::string &key_str = std::string(), int context = 0,
            bool isdouble = false);

    RefKey find(int type_, unsigned int mod_, unsigned int key_,
                int context_, bool isdouble_) {
        // t_key ctor sets context_ of 0 to GLOBAL, so we must here too
        context_ = context_ ? context_ : GLOBAL;
        keylist_t::iterator it = keylist.begin(), it_end = keylist.end();
        for (; it != it_end; ++it) {
            if (*it && (*it)->type == type_ && (*it)->key == key_ &&
                ((*it)->context & context_) > 0 &&
                isdouble_ == (*it)->isdouble && (*it)->mod ==
                FbTk::KeyUtil::instance().isolateModifierMask(mod_))
                return *it;
        }
        return RefKey();
    }

    /**
     * Returns the t_key object corresponding to the binding val and it's parent.
     * The parent comes in handy when we want to remove the binding.
     */
    FindPair findBinding(vector<string> val, bool insert);

    // member variables

    int type; // KeyPress or ButtonPress
    unsigned int mod;
    unsigned int key; // key code or button number
    std::string key_str; // key-symbol, needed for regrab()
    int context; // ON_TITLEBAR, etc.: bitwise-or of all desired contexts
    bool isdouble;
    FbTk::RefCount<FbTk::Command<void> > m_command;

    keylist_t keylist;

    static FbTk::Lua::RegisterInitFunction registerInitKeys;
};

int Keys::t_key::newindex(lua::state *l) {
    l->checkstack(2);

    try {
        l->checkargno(3);

        RefKey k = *l->checkudata<RefKey>(1, keymode_metatable);

        vector<string> val;
        FbTk::StringUtil::stringtok(val, l->checkstring(2).c_str());

        RefKey k2;
        try {
            k2 = *l->checkudata<RefKey>(3, keymode_metatable);
        }
        catch(lua::check_error &) {
            k2.reset(new t_key);

            if(l->isstring(3))
                k2->m_command.reset(FbTk::CommandParser<void>::instance().parse(l->tostring(-1)));
            else if(l->isfunction(3)) {
                l->pushvalue(3);
                k2->m_command.reset(new FbCommands::LuaCmd(*l));
            } else if(l->isnil(3))
                k2.reset();
            else {
                _FB_USES_NLS;
                throw KeyError(_FB_CONSOLETEXT(Keys, Bad3rdArg, "3rd argument is not a command.",
                            "3rd argument is not a command."));
            }
        }

        FindPair p = k->findBinding(val, true);
        if(k2) {
            RefKey t = *p.first;
            k2->type = t->type;
            k2->mod = t->mod;
            k2->key = t->key;
            k2->context = t->context;
            k2->isdouble = t->isdouble;
            *p.first = k2;
        } else
            p.second->keylist.erase(p.first);
    }
    catch(std::runtime_error &e) {
        cerr << "keymode newindex: " << e.what() << endl;
    }

    return 0;
}

int Keys::t_key::newKeyMode(lua::state *l) {
    l->checkstack(2);

    l->createuserdata<RefKey>(new t_key()); {
        l->rawgetfield(lua::REGISTRYINDEX, keymode_metatable);
        l->setmetatable(-2);
    } return 1;
}

int Keys::t_key::index(lua::state *l) {
    l->checkstack(2);

    try {
        l->checkargno(2);

        RefKey k = *l->checkudata<RefKey>(1, keymode_metatable);

        string str = l->checkstring(2);

        if(str == "activate")
            l->pushfunction(&setKeyModeWrapper);
        else if(str == "clear")
            l->pushfunction(&clear);
        else {
            vector<string> val;
            FbTk::StringUtil::stringtok(val, str.c_str());

            FindPair p = k->findBinding(val, false);
            if(p.first == p.second->keylist.end())
                l->pushnil();
            else {
                l->createuserdata<RefKey>(*p.first); {
                    l->rawgetfield(lua::REGISTRYINDEX, keymode_metatable);
                    l->setmetatable(-2);
                }
            }

        }
    }
    catch(std::runtime_error &e) {
        cerr << "keymode index: " << e.what() << endl;
        l->pushnil();
    }

    return 1;
}

int Keys::t_key::clear(lua::state *l) {
    try {
        l->checkargno(1);
        const RefKey &k = *l->checkudata<RefKey>(1, keymode_metatable);

        k->keylist.clear();
        k->m_command.reset();
    }
    catch(std::runtime_error &e) {
        cerr << "clear: " << e.what() << endl;
    }
    return 0;

}

void Keys::t_key::initKeys(FbTk::Lua &l) {
    l.checkstack(3);
    lua::stack_sentry s(l);

    l.newmetatable(keymode_metatable); {
        l.pushdestructor<RefKey>();
        l.rawsetfield(-2, "__gc");

        l.pushfunction(&index);
        l.rawsetfield(-2, "__index");

        l.pushfunction(&newindex);
        l.rawsetfield(-2, "__newindex");
    } l.pop();

    newKeyMode(&l);
    l.readOnlySetField(lua::GLOBALSINDEX, default_keymode);

    l.pushfunction(&newKeyMode);
    l.readOnlySetField(lua::GLOBALSINDEX, "newKeyMode");
}

FbTk::Lua::RegisterInitFunction Keys::t_key::registerInitKeys(&Keys::t_key::initKeys);

Keys::t_key::t_key(int type_, unsigned int mod_, unsigned int key_,
                   const std::string &key_str_,
                   int context_, bool isdouble_) :
    type(type_),
    mod(mod_),
    key(key_),
    key_str(key_str_),
    context(context_ ? context_ : GLOBAL),
    isdouble(isdouble_),
    m_command(0) {
}

Keys::t_key::FindPair Keys::t_key::findBinding(vector<string> val, bool insert ) {

    unsigned int key = 0, mod = 0;
    int type = 0, context = 0;
    bool isdouble = false;
    string processed;
    string key_str;

    // for each argument
    while(!val.empty()) {

        string arg_ = val[0];
        val.erase(val.begin());

        processed += ' ' + arg_;;
        string arg = FbTk::StringUtil::toLower(arg_);

        int tmpmod = FbTk::KeyUtil::getModifier(arg.c_str());
        if(tmpmod)
            mod |= tmpmod; //If it's a modifier
        else if (arg == "ondesktop")
            context |= ON_DESKTOP;
        else if (arg == "ontoolbar")
            context |= ON_TOOLBAR;
        else if (arg == "onwindow")
            context |= ON_WINDOW;
        else if (arg == "ontitlebar")
            context |= ON_TITLEBAR;
        else if (arg == "onwindowborder")
            context |= ON_WINDOWBORDER;
        else if (arg == "onleftgrip")
            context |= ON_LEFTGRIP;
        else if (arg == "onrightgrip")
            context |= ON_RIGHTGRIP;
        else if (arg == "double")
            isdouble = true;
        else {
            if (arg == "focusin") {
                context = ON_WINDOW;
                mod = key = 0;
                type = FocusIn;
            } else if (arg == "focusout") {
                context = ON_WINDOW;
                mod = key = 0;
                type = FocusOut;
            } else if (arg == "changeworkspace") {
                context = ON_DESKTOP;
                mod = key = 0;
                type = FocusIn;
            } else if (arg == "mouseover") {
                type = EnterNotify;
                if (!(context & (ON_WINDOW|ON_TOOLBAR)))
                    context |= ON_WINDOW;
                key = 0;
            } else if (arg == "mouseout") {
                type = LeaveNotify;
                if (!(context & (ON_WINDOW|ON_TOOLBAR)))
                    context |= ON_WINDOW;
                key = 0;

            // check if it's a mouse button
            } else if (extractKeyFromString(arg, "mouse", key)) {
                type = ButtonPress;

                // fluxconf mangles things like OnWindow Mouse# to Mouse#ow
                if (strstr(arg.c_str(), "top"))
                    context = ON_DESKTOP;
                else if (strstr(arg.c_str(), "ebar"))
                    context = ON_TITLEBAR;
                else if (strstr(arg.c_str(), "bar"))
                    context = ON_TOOLBAR;
                else if (strstr(arg.c_str(), "ow"))
                    context = ON_WINDOW;
            } else if (extractKeyFromString(arg, "click", key)) {
                type = ButtonRelease;
            } else if (extractKeyFromString(arg, "move", key)) {
                type = MotionNotify;

            } else if ((key = FbTk::KeyUtil::getKey(arg_.c_str()))) { // convert from string symbol
                type = KeyPress;
                key_str = arg_;

            // keycode covers the following three two-byte cases:
            // 0x       - hex
            // +[1-9]   - number between +1 and +9
            // numbers 10 and above
            //
            } else {
                FbTk::StringUtil::extractNumber(arg, key);
                type = KeyPress;
            }

            break;
        }

    } // end while


    if (key == 0 && (type == 0 || type == KeyPress || type == ButtonPress || type == ButtonRelease))
        throw KeyError("Invalid key combination:" + processed);

    if (type != ButtonPress)
        isdouble = false;

    RefKey new_key = RefKey(new t_key(type, mod, key, key_str, context, isdouble));
    keylist_t::iterator new_it = std::find_if(keylist.begin(), keylist.end(),
            FbTk::MemFun(*new_key, &t_key::equalExact));

    if(new_it == keylist.end() && insert)
        new_it = keylist.insert(new_it, new_key);

    if(new_it == keylist.end() || val.empty())
        return FindPair(new_it, this);
    else
        return (*new_it)->findBinding(val, insert);
}


Keys::Keys():
    m_reloader(new FbTk::AutoReloadHelper(5)),
    m_keylist(0),
    next_key(0), saved_keymode(0) {

    m_reloader->setReloadCmd(FbTk::RefCount<FbTk::Command<void> >(new FbTk::SimpleCommand<Keys>(*this, &Keys::reload)));
    m_reloader->setMainFile(*Fluxbox::instance()->getKeysResource());
    join(Fluxbox::instance()->getKeysResource().modifiedSig(),
            MemFun(*m_reloader, &FbTk::AutoReloadHelper::setMainFile));
}

Keys::~Keys() {
    ungrabKeys();
    ungrabButtons();
    deleteTree();
    delete m_reloader;
}

int Keys::setKeyModeWrapper(lua::state *l) {
    try {
        l->checkargno(1);
        const RefKey &k = *l->checkudata<RefKey>(1, keymode_metatable);
        Fluxbox::instance()->keys()->setKeyMode(k);
    }
    catch(std::runtime_error &e) {
        cerr << "activate: " << e.what() << endl;
    }
    return 0;
}

/// Destroys the keytree
void Keys::deleteTree() {

    m_keylist.reset();
    next_key.reset();
    saved_keymode.reset();
}

// keys are only grabbed in global context
void Keys::grabKey(unsigned int key, unsigned int mod) {
    WindowMap::iterator it = m_window_map.begin();
    WindowMap::iterator it_end = m_window_map.end();

    for (; it != it_end; ++it) {
        if ((it->second & Keys::GLOBAL) > 0)
            FbTk::KeyUtil::grabKey(key, mod, it->first);
    }
}

// keys are only grabbed in global context
void Keys::ungrabKeys() {
    WindowMap::iterator it = m_window_map.begin();
    WindowMap::iterator it_end = m_window_map.end();

    for (; it != it_end; ++it) {
        if ((it->second & Keys::GLOBAL) > 0)
            FbTk::KeyUtil::ungrabKeys(it->first);
    }
}

// ON_DESKTOP context doesn't need to be grabbed
void Keys::grabButton(unsigned int button, unsigned int mod, int context) {
    WindowMap::iterator it = m_window_map.begin();
    WindowMap::iterator it_end = m_window_map.end();

    for (; it != it_end; ++it) {
        if ((context & it->second & ~Keys::ON_DESKTOP) > 0)
            FbTk::KeyUtil::grabButton(button, mod, it->first,
                                      ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);
    }
}

void Keys::ungrabButtons() {
    WindowMap::iterator it = m_window_map.begin();
    WindowMap::iterator it_end = m_window_map.end();

    for (; it != it_end; ++it)
        FbTk::KeyUtil::ungrabButtons(it->first);
}

void Keys::grabWindow(Window win) {
    if (!m_keylist)
        return;

    // make sure the window is in our list
    WindowMap::iterator win_it = m_window_map.find(win);
    if (win_it == m_window_map.end())
        return;

    m_handler_map[win]->grabButtons();
    t_key::keylist_t::iterator it = m_keylist->keylist.begin();
    t_key::keylist_t::iterator it_end = m_keylist->keylist.end();
    for (; it != it_end; ++it) {
        // keys are only grabbed in global context
        if ((win_it->second & Keys::GLOBAL) > 0 && (*it)->type == KeyPress)
            FbTk::KeyUtil::grabKey((*it)->key, (*it)->mod, win);
        // ON_DESKTOP buttons don't need to be grabbed
        else if ((win_it->second & (*it)->context & ~Keys::ON_DESKTOP) > 0) {

            if ((*it)->type == ButtonPress || (*it)->type == ButtonRelease || (*it)->type == MotionNotify) {
                FbTk::KeyUtil::grabButton((*it)->key, (*it)->mod, win, ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);
            }
        }
    }
}

/**
    Load and grab keys
    TODO: error checking
*/
void Keys::reload() {
    // an intentionally empty file will still have one root mapping
    Fluxbox &fluxbox = *Fluxbox::instance();
    FbTk::Lua &l = fluxbox.lua();
    l.checkstack(1);
    lua::stack_sentry s(l);

    deleteTree();

    l.getglobal(default_keymode);
    assert(l.isuserdata(-1));
    RefKey t = *static_cast<RefKey *>(l.touserdata(-1)) = RefKey(new t_key);
    l.pop();

    next_key.reset();
    saved_keymode.reset();

    try {
        l.loadfile(FbTk::StringUtil::expandFilename(*fluxbox.getKeysResource()).c_str());
        l.call(0, 0);
    }
    catch(std::runtime_error &e) {
        _FB_USES_NLS;
        cerr << _FB_CONSOLETEXT(Keys, LoadError, "Error loading keys file: ",
                "Actual error message follows") << e.what() << endl;
        loadDefaults(l);
    }

    setKeyMode(t);
}

/**
 * Load critical key/mouse bindings for when there are fatal errors reading the keyFile.
 */
void Keys::loadDefaults(FbTk::Lua &l) {
    fbdbg<<"Loading default key bindings"<<endl;

    l.loadstring(
        "default_keymode['OnDesktop Mouse1'] = 'HideMenus'\n"
        "default_keymode['OnDesktop Mouse2'] = 'WorkspaceMenu'\n"
        "default_keymode['OnDesktop Mouse3'] = 'RootMenu'\n"
        "default_keymode['OnTitlebar Mouse3'] = 'WindowMenu'\n"
        "default_keymode['Mod1 OnWindow Mouse1'] = 'MacroCmd {Focus} {Raise} {StartMoving}'\n"
        "default_keymode['OnTitlebar Mouse1'] = 'MacroCmd {Focus} {Raise} {ActivateTab}'\n"
        "default_keymode['OnTitlebar Move1'] = 'StartMoving'\n"
        "default_keymode['OnLeftGrip Move1'] = 'StartResizing bottomleft'\n"
        "default_keymode['OnRightGrip Move1'] = 'StartResizing bottomright'\n"
        "default_keymode['OnWindowBorder Move1'] = 'StartMoving'\n"
        "default_keymode['Mod1 Tab'] = 'NextWindow (workspace=[current])'\n"
        "default_keymode['Mod1 Shift Tab'] = 'PrevWindow (workspace=[current])'\n"
    );
    l.call(0, 0);
}

// return true if bound to a command, else false
bool Keys::doAction(int type, unsigned int mods, unsigned int key,
                    int context, WinClient *current, Time time) {

    if (!m_keylist)
        return false;

    static Time last_button_time = 0;
    static unsigned int last_button = 0;

    // need to remember whether or not this is a double-click, e.g. when
    // double-clicking on the titlebar when there's an OnWindow Double command
    // we just don't update it if timestamp is the same
    static bool double_click = false;

    // actual value used for searching
    bool isdouble = false;

    if (type == ButtonPress) {
        if (time > last_button_time) {
            double_click = (time - last_button_time <
                Fluxbox::instance()->getDoubleClickInterval()) &&
                last_button == key;
        }
        last_button_time = time;
        last_button = key;
        isdouble = double_click;
    }

    if (!next_key)
        next_key = m_keylist;

    mods = FbTk::KeyUtil::instance().cleanMods(mods);
    RefKey temp_key = next_key->find(type, mods, key, context, isdouble);

    // just because we double-clicked doesn't mean we shouldn't look for single
    // click commands
    if (!temp_key && isdouble)
        temp_key = next_key->find(type, mods, key, context, false);

    if (temp_key && !temp_key->keylist.empty()) { // emacs-style
        if (!saved_keymode)
            saved_keymode = m_keylist;
        next_key = temp_key;
        setKeyMode(next_key);
        return true;
    }
    if (!temp_key || temp_key->m_command == 0) {
        if (type == KeyPress &&
            !FbTk::KeyUtil::instance().keycodeToModmask(key)) {
            // if we're in the middle of an emacs-style keychain, exit it
            next_key.reset();
            if (saved_keymode) {
                setKeyMode(saved_keymode);
                saved_keymode.reset();
            }
        }
        return false;
    }

    // if focus changes, windows will get NotifyWhileGrabbed,
    // which they tend to ignore
    if (type == KeyPress)
        XUngrabKeyboard(Fluxbox::instance()->display(), CurrentTime);

    WinClient *old = WindowCmd<void>::client();
    WindowCmd<void>::setClient(current);
    temp_key->m_command->execute();
    WindowCmd<void>::setClient(old);

    if (saved_keymode) {
        if (next_key == m_keylist) // don't reset keymode if command changed it
            setKeyMode(saved_keymode);
        saved_keymode.reset();
    }
    next_key.reset();
    return true;
}

/// adds the window to m_window_map, so we know to grab buttons on it
void Keys::registerWindow(Window win, FbTk::EventHandler &h, int context) {
    m_window_map[win] = context;
    m_handler_map[win] = &h;
    grabWindow(win);
}

/// remove the window from the window map, probably being deleted
void Keys::unregisterWindow(Window win) {
    FbTk::KeyUtil::ungrabKeys(win);
    FbTk::KeyUtil::ungrabButtons(win);
    m_handler_map.erase(win);
    m_window_map.erase(win);
}

void Keys::regrab() {
    setKeyMode(m_keylist);
}

void Keys::setKeyMode(const FbTk::RefCount<t_key> &keyMode) {
    ungrabKeys();
    ungrabButtons();

    // notify handlers that their buttons have been ungrabbed
    HandlerMap::iterator h_it = m_handler_map.begin(),
                         h_it_end  = m_handler_map.end();
    for (; h_it != h_it_end; ++h_it)
        h_it->second->grabButtons();

    t_key::keylist_t::iterator it = keyMode->keylist.begin();
    t_key::keylist_t::iterator it_end = keyMode->keylist.end();
    for (; it != it_end; ++it) {
        RefKey t = *it;
        if (t->type == KeyPress) {
            if (!t->key_str.empty()) {
                int key = FbTk::KeyUtil::getKey(t->key_str.c_str());
                t->key = key;
            }
            grabKey(t->key, t->mod);
        } else {
            grabButton(t->key, t->mod, t->context);
        }
    }
    m_keylist = keyMode;
}

