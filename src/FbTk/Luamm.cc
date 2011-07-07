// luamm:  C++ binding for lua
// Copyright (C) 2010 - 2011 Pavel Labath
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


#include <config.h>

#include "Luamm.hh"

namespace lua {
    namespace {
        // keys for storing values in lua registry
        const char cpp_function_metatable [] = "lua::cpp_function_metatable";
        const char lua_exception_namespace[] = "lua::lua_exception_namespace";
        const char this_cpp_object	  [] = "lua::this_cpp_object";

        typedef FbTk::Slot<int, state *> Slot;

        int absindex(lua_State *l, int index) throw()
        { return index<0 && -index<=lua_gettop(l) ? lua_gettop(l)+1+index : index; }

        // Just like getfield(), only without calling metamethods (or throwing random exceptions)
        inline void rawgetfield(lua_State *l, int index, const char *k) throw(std::bad_alloc)
        {
            index = absindex(l, index);
            if(not lua_checkstack(l, 1))
                throw std::bad_alloc();

            lua_pushstring(l, k);
            lua_rawget(l, index);
        }

        // Just like setfield(), only without calling metamethods (or throwing random exceptions)
        inline void rawsetfield(lua_State *l, int index, const char *k) throw(std::bad_alloc)
        {
            index = absindex(l, index);
            if(not lua_checkstack(l, 2))
                throw std::bad_alloc();

            lua_pushstring(l, k);
            lua_insert(l, -2);
            lua_rawset(l, index);
        }

        template<typename Functor>
        int closure_trampoline(lua_State *l)
        {
            lua_checkstack(l, 2);
            rawgetfield(l, REGISTRYINDEX, this_cpp_object);
            assert(lua_islightuserdata(l, -1));
            state *L = static_cast<state *>( lua_touserdata(l, -1) );
            lua_pop(l, 1);

            try {
                Functor *fn = reinterpret_cast<Functor *>( L->touserdata(lua_upvalueindex(1)) );
                assert(fn);
                return (*fn)(L);
            }
            catch(lua::exception &e) {
                // rethrow lua errors as such
                e.push_lua_error(L);
            }
            catch(std::exception &e) {
                L->pushstring(e.what());
            }
            catch(...) {
                L->pushstring("Unknown exception");
            }

            // lua_error does longjmp(), so destructors for objects in this function will not be
            // called
            return lua_error(l);
        }

        /*
         * This function is called when lua encounters an error outside of any protected
         * environment
         */
        int panic_throw(lua_State *l)
        {
            if(not lua_checkstack(l, 1))
                throw std::bad_alloc();

            rawgetfield(l, REGISTRYINDEX, this_cpp_object);
            assert(lua_islightuserdata(l, -1));
            state *L = static_cast<state *>( lua_touserdata(l, -1) );
            lua_pop(l, 1);

            throw lua::exception(L);
        }

        // protected mode wrappers for various lua functions
        int safe_concat_trampoline(lua_State *l)
        {
            lua_concat(l, lua_gettop(l));
            return 1;
        }

        template<int (*compare)(lua_State *, int, int)>
            int safe_compare_trampoline(lua_State *l)
            {
                int r = compare(l, 1, 2);
                lua_pop(l, 2);
                lua_pushinteger(l, r);
                return 1;
            }

        int safe_gc_trampoline(lua_State *l)
        {
            int what = lua_tointeger(l, -2);
            int data = lua_tointeger(l, -1);
            lua_pop(l, 2);
            lua_pushinteger(l, lua_gc(l, what, data));
            return 1;
        }

        template<void (*misc)(lua_State *, int), int nresults>
            int safe_misc_trampoline(lua_State *l)
            {
                misc(l, 1);
                return nresults;
            }

        int safe_next_trampoline(lua_State *l)
        {
            int r = lua_next(l, 1);
            lua_checkstack(l, 1);
            lua_pushinteger(l, r);
            return r ? 3 : 1;
        }

        struct reader_data {
            const void *s;
            size_t len;
        };

        const char *string_reader(lua_State *, void *data, size_t *size)
        {
            reader_data *d = static_cast<reader_data *>(data);
            *size = d->len;
            d->len = 0;
            return static_cast<const char *>(d->s);
        }

    }

    std::string exception::get_error_msg(state *L)
    {
        static const std::string default_msg("Unknown lua exception");

        try {
            return L->tostring(-1);
        }
        catch(not_string_error &e) {
            return default_msg;
        }
    }

    exception::exception(state *l)
        : std::runtime_error(get_error_msg(l)), L(l), L_valid(l->get_valid())
    {
        L->checkstack(1);

        L->rawgetfield(REGISTRYINDEX, lua_exception_namespace);
        L->insert(-2);
        key = L->ref(-2);
        L->pop(1);
    }

    exception::exception(const exception &other)
        : std::runtime_error(other), L(other.L), L_valid(other.L_valid)
    {
        L->checkstack(2);

        L->rawgetfield(REGISTRYINDEX, lua_exception_namespace);
        L->rawgeti(-1, other.key);
        key = L->ref(-2);
        L->pop(1);
    }

    exception::~exception() throw()
    {
        if(not L or not *L_valid)
            return;
        L->checkstack(1);

        L->rawgetfield(REGISTRYINDEX, lua_exception_namespace);
        L->unref(-1, key);
        L->pop();
    }

    void exception::push_lua_error(state *l)
    {
        if(l != L)
            throw std::runtime_error("Cannot transfer exceptions between different lua contexts");
        l->checkstack(2);

        l->rawgetfield(REGISTRYINDEX, lua_exception_namespace);
        l->rawgeti(-1, key);
        l->replace(-2);
    }

    state::state()
        : cobj(luaL_newstate()), valid(new bool(true))
    {
        if(cobj == NULL) {
            // docs say this can happen only in case of a memory allocation error
            throw std::bad_alloc();
        }

        try {
            // set our panic function
            lua_atpanic(cobj, panic_throw);

            checkstack(2);

            // store a pointer to ourselves
            pushlightuserdata(this);
            rawsetfield(REGISTRYINDEX, this_cpp_object);

            // a metatable for C++ functions callable from lua code
            newmetatable(cpp_function_metatable);
            pushboolean(false);
            rawsetfield(-2, "__metatable");
            pushdestructor<Slot>();
            rawsetfield(-2, "__gc");
            pop();

            // while they're travelling through C++ code, lua exceptions will reside here
            newtable();
            rawsetfield(REGISTRYINDEX, lua_exception_namespace);

            luaL_openlibs(cobj);
        }
        catch(...) {
            *valid = false;
            lua_close(cobj);
            throw;
        }
    }

    void state::call(int nargs, int nresults, int errfunc)
    {
        int r = lua_pcall(cobj, nargs, nresults, errfunc);
        if(r == 0)
            return;

        if(r == LUA_ERRMEM) {
            // memory allocation error, cross your fingers
            throw std::bad_alloc();
        }

        // wrap the lua exception and throw it
        if(r == LUA_ERRERR)
            throw lua::errfunc_error(this);
        else
            throw lua::exception(this);
    }

    void state::checkstack(int extra) throw(std::bad_alloc)
    {
        if(not lua_checkstack(cobj, extra))
            throw std::bad_alloc();
    }

    void state::concat(int n)
    {
        assert(n>=0);
        checkstack(1);
        lua_pushcfunction(cobj, safe_concat_trampoline);
        insert(-n-1);
        call(n, 1, 0);
    }

    bool state::equal(int index1, int index2)
    {
        // avoid pcall overhead in trivial cases
        if( rawequal(index1, index2) )
            return true;

        return safe_compare(&safe_compare_trampoline<lua_equal>, index1, index2);
    }

    int state::gc(int what, int data)
    {
        checkstack(3);
        lua_pushcfunction(cobj, safe_gc_trampoline);
        pushinteger(what);
        pushinteger(data);
        call(2, 1, 0);
        assert(isnumber(-1));
        int r = tointeger(-1);
        pop();
        return r;
    }

    void state::getfield(int index, const char *k)
    {
        checkstack(1);
        index = absindex(index);
        pushstring(k);
        gettable(index);
    }

    void state::gettable(int index)
    {
        checkstack(2);
        pushvalue(index);
        insert(-2);
        lua_pushcfunction(cobj, (&safe_misc_trampoline<&lua_gettable, 1>));
        insert(-3);
        call(2, 1, 0);
    }

    bool state::lessthan(int index1, int index2)
    {
        return safe_compare(&safe_compare_trampoline<&lua_lessthan>, index1, index2);
    }

    void state::loadfile(const char *filename)
        throw(lua::syntax_error, lua::file_error, std::bad_alloc)
        {
            switch(luaL_loadfile(cobj, filename)) {
                case 0:
                    return;
                case LUA_ERRSYNTAX:
                    throw lua::syntax_error(this);
                case LUA_ERRFILE:
                    throw lua::file_error(this);
                case LUA_ERRMEM:
                    throw std::bad_alloc();
                default:
                    assert(0);
            }
        }

    void
    state::loadstring(const char *s, size_t len, const char *chunkname)
                        throw(lua::syntax_error, std::bad_alloc)
    {
        reader_data data = { s, len };

        switch(lua_load(cobj, string_reader, &data, chunkname)) {
            case 0:
                return;
            case LUA_ERRSYNTAX:
                throw lua::syntax_error(this);
            case LUA_ERRMEM:
                throw std::bad_alloc();
            default:
                assert(0);
        }
    }

    bool state::next(int index)
    {
        checkstack(2);
        pushvalue(index);
        insert(-2);
        lua_pushcfunction(cobj, &safe_next_trampoline);
        insert(-3);

        call(2, MULTRET, 0);

        assert(isnumber(-1));
        int r = tointeger(-1);
        pop();
        return r;
    }

    void state::do_pushclosure(int n)
    {
        rawgetfield(REGISTRYINDEX, cpp_function_metatable);
        setmetatable(-2);

        insert(-n-1);
        lua_pushcclosure(cobj, &closure_trampoline<Slot>, n+1);
    }

    void state::pushclosure(int (*fn)(state *), int n)
    {
        pushlightuserdata(reinterpret_cast<void *>(fn));
        insert(-n-1);
        lua_pushcclosure(cobj, &closure_trampoline<int (state *)>, n+1);
    }

    void state::rawgetfield(int index, const char *k) throw(std::bad_alloc)
    { lua::rawgetfield(cobj, index, k); }

    void state::rawsetfield(int index, const char *k) throw(std::bad_alloc)
    { lua::rawsetfield(cobj, index, k); }

    bool state::safe_compare(lua_CFunction trampoline, int index1, int index2)
    {
        // if one of the indexes is invalid, return false
        if(isnone(index1) || isnone(index2))
            return false;

        // convert relative indexes into absolute
        index1 = absindex(index1);
        index2 = absindex(index2);

        checkstack(3);
        lua_pushcfunction(cobj, trampoline);
        pushvalue(index1);
        pushvalue(index2);
        call(2, 1, 0);
        assert(isnumber(-1));
        int r = tointeger(-1);
        pop();
        return r;
    }

    void state::setfield(int index, const char *k)
    {
        checkstack(1);
        index = absindex(index);
        pushstring(k);
        insert(-2);
        settable(index);
    }

    void state::settable(int index)
    {
        checkstack(2);
        pushvalue(index);
        insert(-3);
        lua_pushcfunction(cobj, (&safe_misc_trampoline<&lua_settable, 0>));
        insert(-4);
        call(3, 0, 0);
    }

    std::string state::tostring(int index) throw(lua::not_string_error)
    {
        size_t len;
        const char *str = lua_tolstring(cobj, index, &len);
        if(not str)
            throw not_string_error();
        return std::string(str, len);
    }
}
