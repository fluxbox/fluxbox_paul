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

#ifndef FBTK_LUAMM_HH
#define FBTK_LUAMM_HH

#include <assert.h>
#include <cstring>
#include <memory>
#include <stdexcept>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "RefCount.hh"
#include "Slot.hh"

namespace lua {
    class state;

    typedef lua_Integer integer;
    typedef lua_Number number;

    enum {
        ENVIRONINDEX = LUA_ENVIRONINDEX,
        GLOBALSINDEX = LUA_GLOBALSINDEX,
        REGISTRYINDEX = LUA_REGISTRYINDEX
    };

    enum {
        GCSTOP		 = LUA_GCSTOP,
        GCRESTART	 = LUA_GCRESTART,
        GCCOLLECT	 = LUA_GCCOLLECT,
        GCCOUNT		 = LUA_GCCOUNT,
        GCCOUNTB	 = LUA_GCCOUNTB,
        GCSTEP		 = LUA_GCSTEP,
        GCSETPAUSE	 = LUA_GCSETPAUSE,
        GCSETSTEPMUL = LUA_GCSETSTEPMUL
    };

    enum {
        MULTRET = LUA_MULTRET
    };

    enum Type {
        TBOOLEAN	   = LUA_TBOOLEAN,
        TFUNCTION	   = LUA_TFUNCTION,
        TLIGHTUSERDATA = LUA_TLIGHTUSERDATA,
        TNIL		   = LUA_TNIL,
        TNONE		   = LUA_TNONE,
        TNUMBER		   = LUA_TNUMBER,
        TSTRING		   = LUA_TSTRING,
        TTABLE		   = LUA_TTABLE,
        TTHREAD		   = LUA_TTHREAD,
        TUSERDATA	   = LUA_TUSERDATA
    };

    // we reserve one upvalue for the function pointer
    inline int upvalueindex(int n)
    { return lua_upvalueindex(n+1); }

    /*
     * Lua error()s are wrapped in this class when rethrown into C++ code. what() returns the
     * error message. push_lua_error() pushes the error onto lua stack. The error can only be
     * pushed into the same state it was generated in.
     */
    class exception: public std::runtime_error {
        state *L;
        FbTk::RefCount<const bool> L_valid;
        int key;

        static std::string get_error_msg(state *L);

        exception& operator=(const exception &other); // not implemented

    public:
        explicit exception(state *l);
        exception(const exception &other);
        virtual ~exception() throw();

        void push_lua_error(state *l);
    };

    class not_string_error: public std::runtime_error {
    public:
        not_string_error()
            : std::runtime_error("Cannot convert value to a string")
        {}
    };

    // the name says it all
    class syntax_error: public lua::exception {
    public:
        syntax_error(state *L)
            : lua::exception(L)
        {}
    };

    // loadfile() encountered an error while opening/reading the file
    class file_error: public lua::exception {
    public:
        file_error(state *L)
            : lua::exception(L)
        {}
    };

    // double fault, lua encountered an error while running the error handler function
    class errfunc_error: public lua::exception {
    public:
        errfunc_error(state *L)
            : lua::exception(L)
        {}
    };

    // a fancy wrapper around lua_State
    class state {
        lua_State *cobj;

        // destructor for C++ objects stored as lua userdata
        template<typename T>
        static int destroy_cpp_object(lua_State *l)
        {
            T *ptr = static_cast<T *>(lua_touserdata(l, -1));
            assert(ptr);
            try {
                // throwing exceptions in destructors is a bad idea
                // but we catch (and ignore) them, just in case
                ptr->~T();
            }
            catch(...) {
            }
            return 0;
        }

        bool safe_compare(lua_CFunction trampoline, int index1, int index2);
        void do_pushclosure(int n);

        /**
         * The pointed-to value is true if this object still exists. We need this because the
         * exceptions have to know if they may reference it to remove the saved lua exception. If
         * this object is destroyed then the exception was already collected by the garbage
         * colletor and referencing this would generate a segfault.
         */
        FbTk::RefCount<bool> valid;

    public:
        state();
        ~state() { *valid = false; lua_close(cobj); }

        FbTk::RefCount<const bool> get_valid() const { return valid; }

        /*
         * Lua functions come in three flavours
         * a) functions that never throw an exception
         * b) functions that throw only in case of a memory allocation error
         * c) functions that throw other kinds of errors
         *
         * Calls to type a functions are simply forwarded to the C api.
         * Type c functions are executed in protected mode, to make sure they don't longjmp()
         * over us (and our destructors). This add a certain amount overhead. If you care about
         * performance, try using the raw versions (if possible).
         * Type b functions are not executed in protected mode atm. as memory allocation errors
         * don't happen that often (as opposed to the type c, where the user get deliberately set
         * a metamethod that throws an error). That means those errors will do something
         * undefined, but hopefully that won't be a problem.
         *
         * Semantics are mostly identical to those of the underlying C api. Any deviation is
         * noted in the respective functions comment. The most important difference is that
         * instead of return values, we use exceptions to indicate errors.	The lua and C++
         * exception mechanisms are integrated. That means one can throw a C++ exception and
         * catch it in lua (with pcall). Lua error()s can be caught in C++ as exceptions of type
         * lua::exception.
         */

        // type a, never throw
        int absindex(int index) throw() { return index<0 && -index<=gettop() ? gettop()+1+index : index; }
        bool getmetatable(int index) throw() { return lua_getmetatable(cobj, index); }
        int gettop() throw() { return lua_gettop(cobj); }
        void insert(int index) throw() { lua_insert(cobj, index); }
        bool isboolean(int index) throw() { return lua_isboolean(cobj, index); }
        bool isfunction(int index) throw() { return lua_isfunction(cobj, index); }
        bool islightuserdata(int index) throw() { return lua_islightuserdata(cobj, index); }
        bool isnil(int index) throw() { return lua_isnil(cobj, index); }
        bool isnone(int index) throw() { return lua_isnone(cobj, index); }
        bool isnumber(int index) throw() { return lua_isnumber(cobj, index); }
        bool isstring(int index) throw() { return lua_isstring(cobj, index); }
        bool istable(int index) throw() { return lua_istable(cobj, index); }
        void pop(int n = 1) throw() { lua_pop(cobj, n); }
        void pushboolean(bool b) throw() { lua_pushboolean(cobj, b); }
        void pushinteger(integer n) throw() { lua_pushinteger(cobj, n); }
        void pushlightuserdata(void *p) throw() { lua_pushlightuserdata(cobj, p); }
        void pushnil() throw() { lua_pushnil(cobj); }
        void pushnumber(number n) throw() { lua_pushnumber(cobj, n); }
        void pushvalue(int index) throw() { lua_pushvalue(cobj, index); }
        void rawget(int index) throw() { lua_rawget(cobj, index); }
        void rawgeti(int index, int n) throw() { lua_rawgeti(cobj, index, n); }
        bool rawequal(int index1, int index2) throw() { return lua_rawequal(cobj, index1, index2); }
        void replace(int index) throw() { lua_replace(cobj, index); }
        // lua_setmetatable returns int, but docs don't specify it's meaning :/
        int setmetatable(int index) throw() { return lua_setmetatable(cobj, index); }
        void settop(int index) throw() { return lua_settop(cobj, index); }
        bool toboolean(int index) throw() { return lua_toboolean(cobj, index); }
        integer tointeger(int index) throw() { return lua_tointeger(cobj, index); }
        number tonumber(int index) throw() { return lua_tonumber(cobj, index); }
        void* touserdata(int index) throw() { return lua_touserdata(cobj, index); }
        Type type(int index) throw() { return static_cast<Type>(lua_type(cobj, index)); }
        // typename is a reserved word :/
        const char* type_name(Type tp) throw() { return lua_typename(cobj, tp); }
        void unref(int t, int ref) throw() { return luaL_unref(cobj, t, ref); }

        // type b, throw only on memory allocation errors
        // checkstack correctly throws bad_alloc, because lua_checkstack kindly informs us of
        // that sitution
        void checkstack(int extra) throw(std::bad_alloc);
        void createtable(int narr = 0, int nrec = 0) { lua_createtable(cobj, narr, nrec); }
        const char* gsub(const char *s, const char *p, const char *r) { return luaL_gsub(cobj, s, p, r); }
        bool newmetatable(const char *tname) { return luaL_newmetatable(cobj, tname); }
        void newtable() { lua_newtable(cobj); }
        void *newuserdata(size_t size) { return lua_newuserdata(cobj, size); }
        // Functor can be anything that FbTk::Slot can handle, everything else remains
        // identical. We also provide a specialized, lightweight version for simple functors
        template<typename Functor>
        void pushclosure(const Functor &fn, int n);
        void pushclosure(int (*fn)(state *), int n);
        template<typename Functor>
        void pushfunction(const Functor &fn) { pushclosure(fn, 0); }
        void pushstring(const char *s) { lua_pushstring(cobj, s); }
        void pushstring(const char *s, size_t len) { lua_pushlstring(cobj, s, len); }
        void pushstring(const std::string &s) { lua_pushlstring(cobj, s.c_str(), s.size()); }
        void rawgetfield(int index, const char *k) throw(std::bad_alloc);
        void rawset(int index) { lua_rawset(cobj, index); }
        void rawsetfield(int index, const char *k) throw(std::bad_alloc);
        void rawseti(int index, int n) { lua_rawseti(cobj, index, n); }
        int ref(int t) { return luaL_ref(cobj, t); }
        // len recieves length, if not null. Returned value may contain '\0'
        const char* tocstring(int index, size_t *len = NULL) { return lua_tolstring(cobj, index, len); }
        // Don't use pushclosure() to create a __gc function. The problem is that lua calls them
        // in an unspecified order, and we may end up destroying the object holding the
        // FbTk::Slot before we get a chance to call it. This pushes a function that simply
        // calls ~T when the time comes. Only set it as __gc on userdata of type T.
        template<typename T>
        void pushdestructor() { lua_pushcfunction(cobj, &destroy_cpp_object<T>); }

        // type c, throw everything but the kitchen sink
        // call() is a protected mode call, we don't allow unprotected calls
        void call(int nargs, int nresults, int errfunc = 0);
        void concat(int n);
        bool equal(int index1, int index2);
        int gc(int what, int data);
        void getfield(int index, const char *k);
        void gettable(int index);
        void getglobal(const char *name) { getfield(GLOBALSINDEX, name); }
        bool lessthan(int index1, int index2);
        void loadfile(const char *filename) throw(lua::syntax_error, lua::file_error, std::bad_alloc);
        void loadstring(const char *s, const char *chunkname = NULL) throw(lua::syntax_error, std::bad_alloc) { loadstring(s, strlen(s), chunkname); }
        void loadstring(const char *s, size_t len, const char *chunkname = NULL) throw(lua::syntax_error, std::bad_alloc);
        void loadstring(const std::string &s, const char *chunkname = NULL) throw(lua::syntax_error, std::bad_alloc) { loadstring(s.c_str(), s.length(), chunkname); }
        bool next(int index);
        // register is a reserved word :/
        template<typename Functor>
        void register_fn(const char *name, const Functor &f) { pushfunction(f); setglobal(name); }
        void setfield(int index, const char *k);
        void setglobal(const char *name) { setfield(GLOBALSINDEX, name); }
        void settable(int index);
        // lua_tostring uses NULL to indicate conversion error, since there is no such thing as a
        // NULL std::string, we throw an exception. Returned value may contain '\0'
        std::string tostring(int index) throw(lua::not_string_error);
        // allocate a new lua userdata of appropriate size, and create a object in it
        // pushes the userdata on stack and returns the pointer
        template<typename T>
        T* createuserdata();
        template<typename T, typename Arg1>
        T* createuserdata(const Arg1 &arg1);
        template<typename T, typename Arg1, typename Arg2>
        T* createuserdata(const Arg1 &arg1, const Arg2 &arg2);
        template<typename T, typename Arg1, typename Arg2, typename Arg3>
        T* createuserdata(const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3);
    };

    /*
     * Can be used to automatically pop temporary values off the lua stack on exit from the
     * function/block (e.g. via an exception). It's destructor makes sure the stack contains
     * exactly n items. The constructor initializes n to l.gettop()+n_, but that can be later
     * changed with the overloaded operators. It is an error if stack contains less than n
     * elements at entry into the destructor.
     *
     * Proposed stack discipline for functions is this:
     * - called function always pops parameters off the stack.
     * - if functions returns normally, it's return values are on the stack.
     * - if function throws an exception, there are no return values on the stack.
     * The last point differs from lua C api, which return an error message on the stack. But
     * since we have exception.what() for that, putting the message on the stack is not
     * necessary.
     */
    class stack_sentry: private FbTk::NotCopyable {
        state *L;
        int n;

    public:
        explicit stack_sentry(state &l, int n_ = 0) throw()
            : L(&l), n(l.gettop()+n_)
        { assert(n >= 0); }

        ~stack_sentry()			throw() { assert(L->gettop() >= n); L->settop(n); }

        void operator++()		throw() { ++n; }
        void operator--()		throw() { --n; assert(n >= 0); }
        void operator+=(int n_) throw() { n+=n_; }
        void operator-=(int n_) throw() { n-=n_; assert(n >= 0); }
    };

    template<typename T>
    T* state::createuserdata()
    {
        stack_sentry s(*this);

        void *t = newuserdata(sizeof(T));
        new(t) T;
        ++s;
        return static_cast<T *>(t);
    }

    template<typename T, typename Arg1>
    T* state::createuserdata(const Arg1 &arg1)
    {
        stack_sentry s(*this);

        void *t = newuserdata(sizeof(T));
        new(t) T(arg1);
        ++s;
        return static_cast<T *>(t);
    }

    template<typename T, typename Arg1, typename Arg2>
    T* state::createuserdata(const Arg1 &arg1, const Arg2 &arg2)
    {
        stack_sentry s(*this);

        void *t = newuserdata(sizeof(T));
        new(t) T(arg1, arg2);
        ++s;
        return static_cast<T *>(t);
    }

    template<typename T, typename Arg1, typename Arg2, typename Arg3>
    T* state::createuserdata(const Arg1 &arg1, const Arg2 &arg2, const Arg3 &arg3)
    {
        stack_sentry s(*this);

        void *t = newuserdata(sizeof(T));
        new(t) T(arg1, arg2, arg3);
        ++s;
        return static_cast<T *>(t);
    }

    template<typename Functor>
    void state::pushclosure(const Functor &fn, int n)
    {
        checkstack(2);

        createuserdata<FbTk::SlotImpl<Functor, int, state *> >(fn);
        do_pushclosure(n);
    }
}

#endif // FBTK_LUAMM_HH
