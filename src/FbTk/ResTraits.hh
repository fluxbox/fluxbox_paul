// ResTraits.hh
// Copyright (c) 2011 Pavel Labath (pavelo at centrum dot sk)
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

#ifndef FBTK_RESTRAITS_HH
#define FBTK_RESTRAITS_HH

#include <stdexcept>
#include <vector>

#include "FbString.hh"
#include "Luamm.hh"
#include "StringUtil.hh"

namespace FbTk {

struct ConversionError: public std::runtime_error {
    ConversionError(const std::string &msg) : std::runtime_error(msg) {}
};

/* Classes that "know" how to convert from string and lua representations
 * into corresponding C++ types
 */

template<typename T>
struct IntTraits {
    typedef T Type;
    static std::string toString(T x) { return StringUtil::number2String(x); }
    static void toLua(T x, lua::state &l) { l.pushnumber(x); }

    static T fromString(const std::string &x) {
        T t;
        if(StringUtil::extractNumber(x, t))
            return t;
        throw ConversionError("Cannot convert to integer from '" + x + "'");
    }

    static T fromLua(lua::state &l) {
        lua::stack_sentry s(l, -1);

        if(l.isnumber(-1))
            return static_cast<T>(l.tonumber(-1));
        else if(l.isstring(-1))
            return fromString(l.tostring(-1));
        throw ConversionError( std::string("Cannot convert to integer from lua type ")
                                + l.type_name(l.type(-1)) );
    }
};

struct StringTraits {
    typedef std::string Type;
    static const std::string &toString(const std::string &x) { return x; }
    static void toLua(const std::string &x, lua::state &l) { l.pushstring(x); }
    static const std::string &fromString(const std::string &x) { return x; }

    static std::string fromLua(lua::state &l) {
        lua::stack_sentry s(l, -1);

        if(l.isstring(-1) || l.isnumber(-1))
            return l.tostring(-1);
        throw ConversionError( std::string("Cannot convert to string from lua type ")
                                + l.type_name(l.type(-1)) );
    }
};

struct FbStringTraits {
    typedef FbString Type;
    static std::string toString(const FbString &x) { return FbStringUtil::FbStrToLocale(x); }
    static void toLua(const FbString &x, lua::state &l) { l.pushstring(toString(x)); }
    static FbString fromString(const std::string &x) { return FbStringUtil::LocaleStrToFb(x); }

    static FbString fromLua(lua::state &l) {
        lua::stack_sentry s(l, -1);

        if(l.isstring(-1) || l.isnumber(-1))
            return fromString(l.tostring(-1));
        throw ConversionError( std::string("Cannot convert to string from lua type ")
                                + l.type_name(l.type(-1)) );
    }
};

struct BoolTraits {
    typedef bool Type;
    static std::string toString(bool x) { return x ? "true" : "false"; }
    static void toLua(bool x, lua::state &l) { l.pushboolean(x); }
    static bool fromString(const std::string &x) {
        return strcasecmp(x.c_str(), "true") == 0;
    }

    static bool fromLua(lua::state &l) {
        lua::stack_sentry s(l, -1);

        if(l.isstring(-1))
            return fromString(l.tostring(-1));
        else if(l.isnumber(-1))
            return l.tointeger(-1) != 0;
        else
            return l.toboolean(-1);
    }
};

/**
 * To use this class, one must first define a mapping between enum values and their names using
 * the s_map array. A NULL value for name signals the end of the array. E.g.,
 * 
 * template<>
 * EnumTraits<Foo>::Pair EnumTraits<Foo>::s_map[] = { {"Bar", Bar}, {"Baz", Baz} {NULL, Baz} };
 */
template<typename T>
struct EnumTraits {
    typedef T Type;
    struct Pair {
        const char *name;
        T value;
    };
    static const Pair s_map[];

    static std::string toString(T x) {
        for(const Pair *p = s_map; p->name; ++p) {
            if(p->value == x)
                return p->name;
        }
        throw ConversionError("Unknown value for enum");
    }

    static void toLua(T x, lua::state &l) { l.pushstring(toString(x)); }

    static T fromString(const std::string &x) {
        for(const Pair *p = s_map; p->name; ++p) {
            if(strcasecmp(p->name, x.c_str()) == 0)
                return p->value;
        }
        throw ConversionError("Cannot convert to enum from '" + x + "'");
    }

    static T fromLua(lua::state &l) {
        lua::stack_sentry s(l, -1);

        if(l.isstring(-1) || l.isnumber(-1))
            return fromString(l.tostring(-1));
        throw ConversionError( std::string("Cannot convert to enum from lua type ")
                                + l.type_name(l.type(-1)) );
    }
};

template<typename Traits>
struct VectorTraits {
    typedef std::vector<typename Traits::Type> Type;

    VectorTraits(const std::string &delim) : m_delim(delim) {}

    std::string toString(const Type &x) const {
        std::string retval;
        for(size_t i = 0; i < x.size(); ++i) {
            retval += Traits::toString(x[i]);
            retval += m_delim[0];
        }

        return retval;
    }

    static void toLua(const Type &x, lua::state &l) {
        l.checkstack(2);
        l.createtable(x.size());
        lua::stack_sentry s(l);

        for(size_t i = 0; i < x.size(); ++i) {
            Traits::toLua(x[i], l);
            l.rawseti(-2, i+1);
        }
    }

    Type fromString(const std::string &x) const {
        std::vector<std::string> val;
        StringUtil::stringtok(val, x, m_delim.c_str());
        Type retval;

        for(size_t i = 0; i < val.size(); i++) {
            try {
                retval.push_back(Traits::fromString(val[i]));
            }
            catch(std::runtime_error &) {
            }
        }

        return retval;
    }

    static Type fromLua(lua::state &l) {
        l.checkstack(1);
        lua::stack_sentry s(l, -1);
        Type retval;

        if(l.type(-1) == lua::TTABLE) {
            for(size_t i = 1; l.rawgeti(-1, i), !l.isnil(-1); ++i) {
                try {
                    retval.push_back(Traits::fromLua(l));
                }
                catch(std::runtime_error &) {
                }
            }
            return retval;
        }
        throw ConversionError( std::string("Cannot convert to vector from lua type ")
                                + l.type_name(l.type(-1)) );
    }

private:
    std::string m_delim;
};

} // end namespace FbTk

#endif // FBTK_RESTRAITS_HH
