// MenuConvertor.cc for Fluxbox
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

#include "MenuConvertor.hh"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "FbTk/FileUtil.hh"
#include "FbTk/I18n.hh"
#include "FbTk/Luamm.hh"
#include "FbTk/StringUtil.hh"

#include "FbMenuParser.hh"

using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::list;
using std::less;

list<string> MenuConvertor::encoding_stack;
list<size_t> MenuConvertor::stacksize_stack;

FbTk::StringConvertor MenuConvertor::m_stringconvertor(FbTk::StringConvertor::ToFbString);

namespace {

void translateMenuItem(FbMenuParser &parse, Menu &item, Menu &parent,
                       FbTk::StringConvertor &labelconvertor);


void parseMenu(FbMenuParser &pars, Menu &menu,
               FbTk::StringConvertor &label_convertor) {

    while (!pars.eof()) {
        Menu t;
        t.load(pars, label_convertor);
        if (t.key() == "end")
            return;
        translateMenuItem(pars, t, menu, label_convertor);
    }
}

void translateMenuItem(FbMenuParser &parse, Menu &pitem, Menu &parent,
                       FbTk::StringConvertor &labelconvertor) {

    string &str_key = pitem.key();
    string &str_cmd = pitem.command();
    string &str_label = pitem.label();

    if (str_key == "nop") {
    } else if (str_key == "icons") {
    } else if (str_key == "exit") {
    } else if (str_key == "exec") {
    } else if (str_key == "macrocmd") {
        str_cmd = "macrocmd " + str_cmd;
        str_key = "command";
    } else if (str_key == "style") {
    } else if (str_key == "config") {
    } else if (str_key == "include") { // include

        // this will make sure we dont get stuck in a loop
        static size_t safe_counter = 0;
        if (safe_counter > 10)
            return;

        safe_counter++;

        string newfile = FbTk::StringUtil::expandFilename(str_label);
        if (FbTk::FileUtil::isDirectory(newfile.c_str())) {
            // inject every file in this directory into the current menu
            FbTk::Directory dir(newfile.c_str());

            vector<string> filelist(dir.entries());
            for (size_t file_index = 0; file_index < dir.entries(); ++file_index)
                filelist[file_index] = dir.readFilename();
            sort(filelist.begin(), filelist.end(), less<string>());

            for (size_t file_index = 0; file_index < dir.entries(); file_index++) {
                string thisfile(newfile + '/' + filelist[file_index]);

                if (FbTk::FileUtil::isRegularFile(thisfile.c_str()) &&
                        (filelist[file_index][0] != '.') &&
                        (thisfile[thisfile.length() - 1] != '~')) {
                    MenuConvertor::createFromFile(thisfile, parent, false);
                }
            }

        } else {
            // inject this file into the current menu
            MenuConvertor::createFromFile(newfile, parent, false);
        }

        safe_counter--;
        str_key.clear();

    } // end of include
    else if (str_key == "begin")
        str_key.clear();
    else if (str_key == "submenu") {
        str_key = "menu";

        parseMenu(parse, pitem, labelconvertor);
    } else if (str_key == "stylesdir" || str_key == "stylesmenu"
            || str_key == "themesmenu" || str_key == "themesdir") {
        str_cmd = str_label;
        str_label.clear();
        str_key = "stylesdir";
    }
    else if (str_key == "wallpapers" || str_key == "wallpapermenu" ||
             str_key == "rootcommands") {
        pitem.program = str_cmd;
        str_cmd.clear();
        str_key = "wallpapers";
    }
    else if (str_key == "workspaces") {
    } else if (str_key == "separator") {
    } else if (str_key == "encoding") {
        MenuConvertor::startEncoding(str_cmd);
        str_key.clear();
    } else if (str_key == "endencoding") {
        MenuConvertor::endEncoding();
        str_key.clear();
    } else if (str_key == "shade") {
    } else if (str_key == "maximize") {
    } else if (str_key == "iconify") {
    } else if (str_key == "close") {
    } else if (str_key == "kill" || str_key == "killwindow") {
    } else if (str_key == "lower") {
    } else if (str_key == "raise") {
    } else if (str_key == "stick") {
    } else if (str_key == "settitledialog") {
    } else if (str_key == "alpha") {
    } else if (str_key == "extramenus") {
    } else if (str_key == "sendto") {
    } else if (str_key == "layer") {
    } else {
        str_cmd = str_key;
        str_key = "command";
    }

    parent.entries.push_back(pitem);
}

bool getStart(FbMenuParser &parser, string &label, FbTk::StringConvertor &labelconvertor) {
    Menu pitem;
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

} // end of anonymous namespace

void Menu::write(std::ostream &o, unsigned level) {
    if(key().empty())
        return;

    const bool menu = key() == "menu";
    char separator = menu ? '\n': ' ';
    std::string indent(menu ? (level+1)*4 : 0, ' ');

    if(level == 0)
        o << "return ";

    o << string(level*4, ' ') << '{' << separator;
    o << indent << "type = " << lua::quote(key()) << ';' << separator;
    if(! label().empty())
        o << indent << "label = " << lua::quote(label()) << ';' << separator;
    if(! command().empty()) {
        o << indent << (menu ? "title = " : "param = ")
          << lua::quote(command()) << ';' << separator;
    }
    if(! program.empty())
        o << indent << "program = " << lua::quote(program) << ';' << separator;
    if(! icon().empty())
        o << indent << "icon = " << lua::quote(icon()) << ';' << separator;

    for(std::list<Menu>::iterator it = entries.begin(); it != entries.end(); ++it) {
        it->write(o, level + 1);
    }

    o << (menu ? string(level*4, ' ') : "") << "};" << endl;
}

bool MenuConvertor::createFromFile(const string &filename,
                                 Menu &inject_into,
                                 bool begin) {
    string real_filename = FbTk::StringUtil::expandFilename(filename);

    FbMenuParser parser(real_filename);
    if (!parser.isLoaded())
        return false;

    startFile();
    if (begin) {
        if (!getStart(parser, inject_into.label(), m_stringconvertor)) {
            endFile();
            return false;
        }
        inject_into.key() = "menu";
    }

    parseMenu(parser, inject_into, m_stringconvertor);
    endFile();

    return true;
}

/* push our encoding-stacksize onto the stack */
void MenuConvertor::startFile() {
    if (encoding_stack.empty())
        m_stringconvertor.setSource("");
    stacksize_stack.push_back(encoding_stack.size());
}

/**
 * Pop necessary encodings from the stack
 * (and endEncoding the final one) to our matching encoding-stacksize.
 */
void MenuConvertor::endFile() {
    size_t target_size = stacksize_stack.back();
    size_t curr_size = encoding_stack.size();

    if (target_size != curr_size) {
        _FB_USES_NLS;
        cerr<<_FB_CONSOLETEXT(Menu, ErrorEndEncoding, "Warning: unbalanced [encoding] tags", "User menu file had unbalanced [encoding] tags")<<endl;
    }

    for (; curr_size > (target_size+1); --curr_size)
        encoding_stack.pop_back();

    if (curr_size == (target_size+1))
        endEncoding();

    stacksize_stack.pop_back();
}

/**
 * Push the encoding onto the stack, and make it active.
 */
void MenuConvertor::startEncoding(const string &encoding) {
    // we push it regardless of whether it's valid, since we
    // need to stay balanced with the endEncodings.
    encoding_stack.push_back(encoding);

    // this won't change if it doesn't succeed
    m_stringconvertor.setSource(encoding);
}

/**
 * Pop the encoding from the stack, unless we are at our stacksize limit.
 * Restore the previous (valid) encoding.
 */
void MenuConvertor::endEncoding() {
    size_t min_size = stacksize_stack.back();
    if (encoding_stack.size() <= min_size) {
        _FB_USES_NLS;
        cerr<<_FB_CONSOLETEXT(Menu, ErrorEndEncoding, "Warning: unbalanced [encoding] tags", "User menu file had unbalanced [encoding] tags")<<endl;
        return;
    }

    encoding_stack.pop_back();
    m_stringconvertor.reset();

    list<string>::reverse_iterator it = encoding_stack.rbegin();
    list<string>::reverse_iterator it_end = encoding_stack.rend();
    while (it != it_end && !m_stringconvertor.setSource(*it))
        ++it;

    if (it == it_end)
        m_stringconvertor.setSource("");
}

