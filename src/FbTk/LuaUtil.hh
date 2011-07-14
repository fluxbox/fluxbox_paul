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

#ifndef FBTK_LUAUTIL_HH
#define FBTK_LUAUTIL_HH

#include <vector>

#include "Luamm.hh"

namespace FbTk {

/*
 * This class augments lua::state with additional functions/features. Every object automatically
 * calls registered init functions, which can be used to initialize/create global variables in
 * the lua state. Functions can be registered by calling registerInitFunction or by creating
 * objects of type RegisterInitFunction.
 */
class Lua: public lua::state {
public:
    Lua();

    /* 
     * makeReadOnly() makes the table at the specified index "read only". This means that any
     * attempt to modify a table entry will result in an error (if only_existing_fields is
     * false). If only_existing_fields is true then only fields that were present at the time of
     * the call will be protected - user can add new entries and modify them afterwards. You
     * should avoid raw access to "read only" tables -- it might not do what you think it will.
     */
    void makeReadOnly(int index, bool only_existing_fields = false);

    /* 
     * readOnlySet() is the equivalent of settable, except that it works on "read only" tables.
     * It can be used to modify protected entries or create new ones.
     */
    void readOnlySet(int index);

    template<typename Functor>
    static void registerInitFunction(const Functor &fn) {
        s_init_functions.push_back(new SlotImpl<Functor, void, Lua &>(fn));
    }

    class RegisterInitFunction {
    public:
        template<typename Functor>
        RegisterInitFunction(const Functor &fn) {
            registerInitFunction(fn);
        }
    };

private:
    typedef Slot<void, Lua &> InitFunction;
    struct AutoVector: public std::vector<InitFunction *> {
        ~AutoVector() {
            for(iterator it = begin(); it != end(); ++it)
                delete *it;
        }
    };
    typedef AutoVector InitFunctions;

    static InitFunctions s_init_functions;
};

} // namespace FbTk

#endif // FBTK_LUAUTIL_HH
