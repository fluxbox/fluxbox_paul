// MenuConvertor.hh for Fluxbox
// Copyright (c) 2004 Henrik Kinnunen (fluxgen at fluxbox dot org)
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

#ifndef MENUCREATOR_HH
#define MENUCREATOR_HH

#include <list>

#include "FbTk/FbString.hh"

#include "FbMenuParser.hh"

class Menu {
public:
    void load(FbMenuParser &p, FbTk::StringConvertor &m_labelconvertor) {
        p>>m_key>>m_label>>m_cmd>>m_icon;
        m_label.second = m_labelconvertor.recode(m_label.second);
    }
    std::string &icon() { return m_icon.second; }
    std::string &command() { return m_cmd.second; }
    std::string &label() { return m_label.second; }
    std::string &key() { return m_key.second; }

    std::list<Menu> entries; // for submenus
    std::string program; // for wallpaper menu entries

    void write(std::ostream &o, unsigned level = 0);
private:
    FbMenuParser::Item m_key, m_label, m_cmd, m_icon;
};

class MenuConvertor {
public:
    static bool createFromFile(const std::string &filename,
                               Menu &inject_into,
                               bool begin = true);

    /**
     * Encoding-related helpers (encoding, aka codeset)
     */

    // Files are guaranteed to be "balanced", unlike user-created [encoding] tags.
    static void startFile();
    static void endFile();

    static void startEncoding(const std::string &encoding);
    static void endEncoding();

private:
    // stack of encodings
    static std::list<std::string> encoding_stack;
    // stack of ints, representing stack size as each file is entered
    // (a file should never end more encodings than it starts)
    static std::list<size_t> stacksize_stack;

    static FbTk::StringConvertor m_stringconvertor;
};

#endif // MENUCREATOR_HH
