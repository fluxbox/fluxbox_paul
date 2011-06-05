// Resources.cc for Fluxbox Window Manager
// Copyright (c) 2004 - 2006 Henrik Kinnunen (fluxgen at fluxbox dot org)
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

// holds main resource functions

#include "FbTk/StringUtil.hh"
#include "FbTk/Resource.hh"
#include "FbTk/Luamm.hh"
#include "WinButton.hh"

#include "fluxbox.hh"

#include "Layer.hh"

#include <stdio.h>
#include <string>
#include <vector>
#ifdef HAVE_CSTRING
  #include <cstring>
#else
  #include <string.h>
#endif

using std::string;
using std::vector;

//-----------------------------------------------------------------
//---- accessors for int, bool, and some enums with Resource ------
//-----------------------------------------------------------------
namespace FbTk {

template<>
string FbTk::Resource<int>::
getString() const {
    return FbTk::StringUtil::number2String(**this);
}

template<>
void FbTk::Resource<int>::
setFromString(const char* strval) {
    FbTk::StringUtil::extractNumber(strval, get());
}

template<>
void FbTk::Resource<int>::setFromLua(lua::state &l) {
    lua::stack_sentry s(l, -1);
    if(l.isnumber(-1))
        *this = l.tonumber(-1);
    else if(l.isstring(-1))
        setFromString(l.tostring(-1).c_str());
    l.pop();
}

template<>
void FbTk::Resource<int>::pushToLua(lua::state &l) const {
    l.pushnumber(*this);
}


template<>
string FbTk::Resource<string>::
getString() const { return **this; }

template<>
void FbTk::Resource<string>::
setFromString(const char *strval) {
    *this = strval;
}

template<>
void FbTk::Resource<string>::setFromLua(lua::state &l) {
    lua::stack_sentry s(l, -1);
    if(l.isstring(-1))
        *this = l.tostring(-1);
    l.pop();
}

template<>
void FbTk::Resource<string>::pushToLua(lua::state &l) const {
    l.pushstring(*this);
}


template<>
string FbTk::Resource<bool>::
getString() const {
    return string(**this == true ? "true" : "false");
}

template<>
void FbTk::Resource<bool>::
setFromString(char const *strval) {
    *this = (bool)!strcasecmp(strval, "true");
}

template<>
void FbTk::Resource<bool>::setFromLua(lua::state &l) {
    lua::stack_sentry s(l, -1);
    if(l.isstring(-1))
        setFromString(l.tostring(-1).c_str());
    else if(l.isnumber(-1))
        *this = l.tointeger(-1) != 0;
    else
        *this = l.toboolean(-1);
    l.pop();
}

template<>
void FbTk::Resource<bool>::pushToLua(lua::state &l) const {
    l.pushboolean(*this);
}


namespace {
    struct ButtonPair {
        WinButton::Type type;
        const char *name;
    };
    const ButtonPair button_map[] = {
        { WinButton::SHADE, "Shade" },
        { WinButton::MINIMIZE, "Minimize" },
        { WinButton::MAXIMIZE, "Maximize" },
        { WinButton::CLOSE, "Close" },
        { WinButton::STICK, "Stick" },
        { WinButton::MENUICON, "MenuIcon" }
    };

    const char *buttonToStr(WinButton::Type t) {
        for(size_t i = 0; i < sizeof(button_map)/sizeof(button_map[0]); ++i) {
            if(button_map[i].type == t)
                return button_map[i].name;
        }
        assert(0);
    }

    WinButton::Type strToButton(const char *v) {
        for(size_t i = 0; i < sizeof(button_map)/sizeof(button_map[0]); ++i) {
            if(strcasecmp(v, button_map[i].name) == 0)
                return button_map[i].type;
        }
        throw std::runtime_error("bad button");
    }
}

template<>
string FbTk::Resource<vector<WinButton::Type> >::
getString() const {
    string retval;
    for (size_t i = 0; i < m_value.size(); i++) {
        retval.append(buttonToStr(m_value[i]));
        retval.append(" ");
    }

    return retval;
}

template<>
void FbTk::Resource<vector<WinButton::Type> >::
setFromString(char const *strval) {
    vector<string> val;
    StringUtil::stringtok(val, strval);
    //clear old values
    m_value.clear();

    for (size_t i = 0; i < val.size(); i++) {
        try {
            m_value.push_back(strToButton(val[i].c_str()));
        }
        catch(std::runtime_error &) {
        }
    }
}

template<>
void FbTk::Resource<vector<WinButton::Type> >::setFromLua(lua::state &l) {
    l.checkstack(1);
    lua::stack_sentry s(l, -1);

    if(l.type(-1) == lua::TTABLE) {
        for(size_t i = 0; l.rawgeti(-1, i), !l.isnil(-1); l.pop(), ++i) {
            if(l.isstring(-1)) {
                try {
                    m_value.push_back(strToButton(l.tostring(-1).c_str()));
                }
                catch(std::runtime_error &) {
                }
            }
        }
        l.pop();
    }
    l.pop();
}

template<>
void FbTk::Resource<vector<WinButton::Type> >::pushToLua(lua::state &l) const {
    l.checkstack(2);
    l.newtable();
    lua::stack_sentry s(l);

    for (size_t i = 0; i < m_value.size(); ++i) {
        l.pushstring(buttonToStr(m_value[i]));
        l.rawseti(-2, i);
    }
}


template<>
string FbTk::Resource<Fluxbox::TabsAttachArea>::
getString() const {
    if (m_value == Fluxbox::ATTACH_AREA_TITLEBAR)
        return "Titlebar";
    else
        return "Window";
}

template<>
void FbTk::Resource<Fluxbox::TabsAttachArea>::
setFromString(char const *strval) {
    if (strcasecmp(strval, "Titlebar")==0)
        m_value= Fluxbox::ATTACH_AREA_TITLEBAR;
    else
        m_value= Fluxbox::ATTACH_AREA_WINDOW;
}

template<>
void FbTk::Resource<Fluxbox::TabsAttachArea>::setFromLua(lua::state &l) {
    lua::stack_sentry s(l, -1);

    if(l.isstring(-1) && strcasecmp(l.tostring(-1).c_str(), "Titlebar") == 0)
        m_value = Fluxbox::ATTACH_AREA_TITLEBAR;
    else
        m_value = Fluxbox::ATTACH_AREA_WINDOW;

    l.pop();
}

template<>
void FbTk::Resource<Fluxbox::TabsAttachArea>::pushToLua(lua::state &l) const {
    l.checkstack(1);

    l.pushstring(getString());
}


template<>
string FbTk::Resource<unsigned int>::
getString() const {
    return FbTk::StringUtil::number2String(m_value);
}

template<>
void FbTk::Resource<unsigned int>::
setFromString(const char *strval) {
    if (!FbTk::StringUtil::extractNumber(strval, m_value))
        setDefaultValue();
}

template<>
void FbTk::Resource<unsigned int>::setFromLua(lua::state &l) {
    lua::stack_sentry s(l, -1);
    if(l.isnumber(-1))
        *this = l.tonumber(-1);
    else if(l.isstring(-1))
        setFromString(l.tostring(-1).c_str());
    l.pop();
}

template<>
void FbTk::Resource<unsigned int>::pushToLua(lua::state &l) const {
    l.pushnumber(*this);
}


template<>
string FbTk::Resource<long long>::
getString() const {
    return FbTk::StringUtil::number2String(m_value);
}

template<>
void FbTk::Resource<long long>::
setFromString(const char *strval) {
    if (!FbTk::StringUtil::extractNumber(strval, m_value))
        setDefaultValue();
}

template<>
void FbTk::Resource<long long>::setFromLua(lua::state &l) {
    lua::stack_sentry s(l, -1);
    if(l.isnumber(-1))
        *this = l.tonumber(-1);
    else if(l.isstring(-1))
        setFromString(l.tostring(-1).c_str());
    l.pop();
}

template<>
void FbTk::Resource<long long>::pushToLua(lua::state &l) const {
    l.pushnumber(*this);
}


template<>
string FbTk::Resource<ResourceLayer>::
getString() const {
    return ::ResourceLayer::getString(m_value.getNum());
}

template<>
void FbTk::Resource<ResourceLayer>::
setFromString(const char *strval) {
    string str(strval);
    int tempnum = ::ResourceLayer::getNumFromString(str);
    if (tempnum >= 0 && tempnum < ::ResourceLayer::NUM_LAYERS)
        m_value = tempnum;
    else
        setDefaultValue();
}

template<>
void FbTk::Resource<ResourceLayer>::setFromLua(lua::state &l) {
    lua::stack_sentry s(l, -1);

    int tempnum = -1;
    if(l.isnumber(-1))
        tempnum = l.tonumber(-1);
    else if(l.isstring(-1))
        tempnum = ::ResourceLayer::getNumFromString(l.tostring(-1));

    if (tempnum >= 0 && tempnum < ::ResourceLayer::NUM_LAYERS)
        m_value = tempnum;
    else
        setDefaultValue();

    l.pop();
}

template<>
void FbTk::Resource<ResourceLayer>::pushToLua(lua::state &l) const {
    l.pushstring(getString());
}


template<>
string FbTk::Resource<long>::
getString() const {
    return FbTk::StringUtil::number2String(m_value);
}

template<>
void FbTk::Resource<long>::
setFromString(const char *strval) {
    if (!FbTk::StringUtil::extractNumber(strval, m_value))
        setDefaultValue();
}

template<>
void FbTk::Resource<long>::setFromLua(lua::state &l) {
    lua::stack_sentry s(l, -1);
    if(l.isnumber(-1))
        *this = l.tonumber(-1);
    else if(l.isstring(-1))
        setFromString(l.tostring(-1).c_str());
    l.pop();
}

template<>
void FbTk::Resource<long>::pushToLua(lua::state &l) const {
    l.pushnumber(*this);
}

} // end namespace FbTk
