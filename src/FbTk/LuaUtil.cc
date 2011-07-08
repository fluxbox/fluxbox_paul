// LuaUtil: Various additional functions for working with lua
// Copyright (C) 2011 Pavel Labath
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "LuaUtil.hh"

namespace FbTk {

namespace {
    const char newindexDenyWriteName[] = "FbTk::Lua::newindexDenyWrite";
    const char newindexDenyModifyName[] = "FbTk::Lua::newindexDenyModify";

    int newindexDenyWrite(lua::state *l) {
        if(l->isstring(-2))
            throw std::runtime_error("Cannot modify field '" + l->tostring(-2) + "'.");
        else
            throw std::runtime_error("Cannot modify this field.");
    }

    int newindexDenyModify(lua::state *l) {
        bool ok = false;
        l->getmetatable(-3); {
            l->rawgetfield(-1, "__index"); {
                l->pushvalue(-4); l->rawget(-2); {
                    if(l->isnil(-1))
                        ok = true;
                } l->pop();
            } l->pop();
        } l->pop();

        if(ok)
            l->rawset(-3);
        else 
            newindexDenyWrite(l);

        return 0;
    }

} // anonymous namespace

Lua::InitFunctions Lua::s_init_functions;

Lua::Lua() {
    checkstack(1);
    lua::stack_sentry s(*this);

    pushfunction(&newindexDenyWrite);
    rawsetfield(lua::REGISTRYINDEX, newindexDenyWriteName);

    pushfunction(&newindexDenyModify);
    rawsetfield(lua::REGISTRYINDEX, newindexDenyModifyName);

    makeReadOnly(lua::GLOBALSINDEX, true);

    InitFunctions::const_iterator it_end = s_init_functions.end();
    for(InitFunctions::const_iterator it = s_init_functions.begin(); it != it_end; ++it)
        (**it)(*this);
}

void Lua::makeReadOnly(int index, bool only_existing_fields) {
    checkstack(6);
    lua::stack_sentry s(*this);
    index = absindex(index);

    newtable(); {
        newtable(); {
            pushnil(); while(next(index)) {
                pushvalue(-2); pushvalue(-2); rawset(-5);

                pop(); pushnil(); rawset(index);

                pushnil();
            }
        } rawsetfield(-2, "__index");

        rawgetfield(lua::REGISTRYINDEX,
                only_existing_fields ? newindexDenyModifyName : newindexDenyWriteName);
        rawsetfield(-2, "__newindex");

        pushboolean(false);
        rawsetfield(-2, "__metatable");
    } setmetatable(index);
}

void Lua::readOnlySet(int index) {
    checkstack(2);
    lua::stack_sentry s(*this, -2);

    getmetatable(index); {
        rawgetfield(-1, "__index"); insert(-4);
    } pop();

    rawset(-3);
    pop();
}

} // namespace FbTk
