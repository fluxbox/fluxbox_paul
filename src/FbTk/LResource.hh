// LResource: Fluxbox Resource implementation in lua
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

#ifndef FBTK_LRESOURCE_HH
#define FBTK_LRESOURCE_HH

#include <cassert>
#include <list>
#include <memory>
#include <string>

#include "Resource.hh"

namespace FbTk {

class Lua;

class LResourceManager: public ResourceManager_base {
public:
    static void convert(ResourceManager &old, const std::string &new_file);

    LResourceManager(const std::string &root, Lua &l);
    virtual bool save(const char *filename, const char *);
    virtual void addResource(Resource_base &r);
    virtual void removeResource(Resource_base &r);
    void setLua(Lua &l);

private:
    void doAddResource(Resource_base &r);
    void doRemoveResource(Resource_base &r);

    Lua *m_l;
};

} // end namespace FbTk

#endif // FBTK_LRESOURCE_HH
