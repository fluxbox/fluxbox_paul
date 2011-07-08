// LResource: Fluxbox Resource implementation in lua
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

#include "LResource.hh"

#include "LuaUtil.hh"
#include "Resource.hh"

extern const char LResourceHelper[];
extern const unsigned int LResourceHelper_size;

namespace FbTk {

namespace {
    const char make_root[] = "FbTk::make_root";
    const char register_resource[] = "FbTk::register_resource";
    const char dump_resources[] = "FbTk::dump_resources";
    const char resource_metatable[] = "FbTk::resource_metatable";

    int readResource(lua::state *l) {
        Resource_base *r = *static_cast<Resource_base **>(l->touserdata(-1));
        l->pop();

        if(r != NULL)
            r->pushToLua(*l);
        else
            l->pushnil();

        return 1;
    }

    int writeResource(lua::state *l) {
        Resource_base *r = *static_cast<Resource_base **>(l->touserdata(-2));
        l->replace(-2);

        if(r != NULL)
            r->setFromLua(*l);
        else
            l->pop();

        return 0;
    }

    void initState(Lua &l) {
        l.checkstack(6);
        lua::stack_sentry s(l);

        l.loadstring(LResourceHelper, LResourceHelper_size);
        l.pushfunction(&readResource);
        l.pushfunction(&writeResource);
        l.newtable(); {
            l.newtable(); {
                l.pushvalue(-2);
                l.setfield(-2, "__metatable");
            } l.setfield(lua::REGISTRYINDEX, resource_metatable);
        }
        l.call(3, 3);
        l.setfield(lua::REGISTRYINDEX, dump_resources);
        l.setfield(lua::REGISTRYINDEX, register_resource);
        l.setfield(lua::REGISTRYINDEX, make_root);
    }

    Lua::RegisterInitFunction register_init_state(&initState);

} // anonymous namespace

void LResourceManager::convert(ResourceManager &old, const std::string &new_file) {
    Lua l;

    LResourceManager new_rm(old.root(), l);
    for(ResourceList::const_iterator i = old.begin(); i != old.end(); ++i) {
        // adding the resource to new_rm will set it to default value
        // we save the value to a temp variable so we can restore it later
        const std::string &t = (*i)->getString();
        new_rm.addResource(**i);
        (*i)->setFromString(t.c_str());
    }

    new_rm.save(new_file.c_str(), NULL);
}

LResourceManager::LResourceManager(const std::string &root, Lua &l)
                        : ResourceManager_base(root), m_l(&l) {
    l.checkstack(2);
    lua::stack_sentry s(l);

    l.getfield(lua::REGISTRYINDEX, make_root);
    l.pushstring(root);
    l.call(1, 0);
}

bool LResourceManager::save(const char *filename, const char *) {
    m_l->checkstack(3);
    lua::stack_sentry s(*m_l);

    m_l->getfield(lua::REGISTRYINDEX, dump_resources);
    m_l->getfield(lua::GLOBALSINDEX, m_root.c_str());
    m_l->pushstring(filename);
    m_l->call(2, 0);

    return true; // FIXME
}

void LResourceManager::addResource(Resource_base &r) {
    m_l->checkstack(5);
    lua::stack_sentry s(*m_l);

    ResourceManager_base::addResource(r);

    m_l->getfield(lua::REGISTRYINDEX, register_resource);
    m_l->getfield(lua::GLOBALSINDEX, m_root.c_str());
    m_l->pushstring(r.name());
    m_l->createuserdata<Resource_base *>(&r); {
        m_l->getfield(lua::REGISTRYINDEX, resource_metatable);
        m_l->setmetatable(-2);
    }
    m_l->call(3, 0);
}

void LResourceManager::removeResource(Resource_base &r) {
    m_l->checkstack(5);
    lua::stack_sentry s(*m_l);

    m_l->getfield(lua::REGISTRYINDEX, register_resource);
    m_l->getfield(lua::GLOBALSINDEX, m_root.c_str());
    m_l->pushstring(r.name());
    r.pushToLua(*m_l);
    m_l->call(3, 1);
    *static_cast<Resource_base **>(m_l->touserdata(-1)) = NULL;
    m_l->pop();

    ResourceManager_base::removeResource(r);
}

} // end namespace FbTk
