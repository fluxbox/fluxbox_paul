// Resource.hh
// Copyright (c) 2002-2003 Henrik Kinnunen (fluxgen at fluxbox dot org)
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

#ifndef FBTK_RESOURCE_HH
#define FBTK_RESOURCE_HH

#include "NotCopyable.hh"
#include "Accessor.hh"

#include <string>
#include <list>
#include <iostream>

#include <exception>
#include <typeinfo>
#include "ResTraits.hh"
#include "XrmDatabaseHelper.hh"

namespace FbTk {

class ResourceException: public std::exception {
public:
    ResourceException(const std::string &err):
        m_str(err) { };
    ~ResourceException() throw() { }
    const char *what() const throw () { return m_str.c_str(); }
private:
    std::string m_str;
};

/// Base class for resources, this is only used in ResourceManager
class Resource_base:private FbTk::NotCopyable
{
public:
    virtual ~Resource_base() { };

    /// set from string value
    virtual void setFromString(char const *strval) = 0;
    /// set default value
    virtual void setDefaultValue() = 0;
    /// get string value
    virtual std::string getString() const = 0;
    /// get alternative name of this resource
    const std::string& altName() const { return m_altname; }
    /// get name of this resource
    const std::string& name() const { return m_name; }

    // Sets the resource value using the value on top of lua stack. Pops the value.
    virtual void setFromLua(lua::state &l) = 0;

    // pushes the value of the resource on the stack
    virtual void pushToLua(lua::state &l) const = 0;

protected:
    Resource_base(const std::string &name, const std::string &altname):
    m_name(name), m_altname(altname)
    { }

private:
    std::string m_name; ///< name of this resource
    std::string m_altname; ///< alternative name
};

template <typename T, typename Traits>
class Resource;

class ResourceManager_base
{
public:
    typedef std::list<Resource_base *> ResourceList;

    ResourceManager_base(const std::string &root) : m_root(root) {}

    virtual ~ResourceManager_base() {}

    /// Save all resouces registered to this class
    /// @return true on success
    virtual bool save(const char *filename, const char *mergefilename=0) = 0;



    /// Add resource to list, only used in Resource<T>
    virtual void addResource(Resource_base &r);

    /// Remove a specific resource, only used in Resource<T>
    virtual void removeResource(Resource_base &r) {
        m_resourcelist.remove(&r);
    }

    /// searches for the resource with the resourcename
    /// @return pointer to resource base on success, else 0.
    Resource_base *findResource(const std::string &resourcename);
    /// searches for the resource with the resourcename
    /// @return pointer to resource base on success, else 0.
    const Resource_base *findResource(const std::string &resourcename) const;

    std::string resourceValue(const std::string &resourcename) const;
    void setResourceValue(const std::string &resourcename, const std::string &value);

    /**
     * Will search and cast the resource to Resource<Type>,
     * it will throw exception if it fails
     * @return reference to resource type
     */
    template <typename ResourceType, typename Traits>
    Resource<ResourceType, Traits> &getResource(const std::string &resource);

    const std::string &root() const { return m_root; }
    ResourceList::const_iterator begin() { return m_resourcelist.begin(); }
    ResourceList::const_iterator end() { return m_resourcelist.end(); }

protected:
    ResourceList m_resourcelist;
    const std::string m_root;
};

class ResourceManager: public ResourceManager_base
{
public:
    // lock specifies if the database should be opened with one level locked
    // (useful for constructing inside initial set of constructors)
    ResourceManager(const std::string &root, const std::string &alt_root,
            const char *filename, bool lock_db);
    virtual ~ResourceManager();

    /// Load all resources registered to this class
    /// @return true on success
    virtual bool load(const char *filename);

    /// Save all resouces registered to this class
    /// @return true on success
    virtual bool save(const char *filename, const char *mergefilename=0);


    /// Add resource to list, only used in Resource<T>
    virtual void addResource(Resource_base &r);

    // this marks the database as "in use" and will avoid reloading
    // resources unless it is zero.
    // It returns this resource manager. Useful for passing to
    // constructors like Object(m_rm.lock())
    ResourceManager &lock();
    void unlock();
    // for debugging
    int lockDepth() const { return m_db_lock; }
    void dump() {
        ResourceList::iterator it = m_resourcelist.begin();
        ResourceList::iterator it_end = m_resourcelist.end();
        for (; it != it_end; ++it) {
            std::cerr<<(*it)->name()<<std::endl;
        }
    }
protected:

    int m_db_lock;

private:

    XrmDatabaseHelper *m_database;

    std::string m_filename;
    std::string m_alt_root;
};


/// Real resource class
/**
 * usage: Resource<int, IntTraits<int> > someresource(resourcemanager, 10, "someresourcename", "somealternativename");
 * If there is no traits class for your type, you have to implement one.
 */
template <typename T, typename Traits>
class Resource:public Resource_base, public Accessor<T> {
public:
    typedef T Type;
    Resource(ResourceManager_base &rm, T val, const std::string &name, const std::string &altname):
        Resource_base(name, altname), m_value(val), m_defaultval(val), m_rm(rm) {
        m_rm.addResource(*this); // add this to resource handler
    }
    virtual ~Resource() {
        m_rm.removeResource(*this); // remove this from resource handler
    }

    void setDefaultValue() {  m_value = m_defaultval; }
    /// sets resource from string, specialized, must be implemented
    void setFromString(const char *strval) {
        try {
            m_value = Traits::fromString(strval);
        }
        catch(ConversionError &e) {
            std::cerr << name() << ": " << e.what() << std::endl;
            setDefaultValue();
        }
    }
    Accessor<T> &operator =(const T& newvalue) { m_value = newvalue; return *this;}
    /// specialized, must be implemented
    /// @return string value of resource
    std::string getString() const { return Traits::toString(m_value); }

    virtual void setFromLua(lua::state &l) {
        try {
            m_value = Traits::fromLua(l);
        }
        catch(ConversionError &e) {
            std::cerr << name() << ": " << e.what() << std::endl;
            setDefaultValue();
        }
    }
    virtual void pushToLua(lua::state &l) const { Traits::toLua(m_value, l); }

    operator T() const { return m_value; }
    T& get() { return m_value; }
    T& operator*() { return m_value; }
    const T& operator*() const { return m_value; }
    T *operator->() { return &m_value; }
    const T *operator->() const { return &m_value; }
private:
    T m_value, m_defaultval;
    ResourceManager_base &m_rm;
};


template <typename ResourceType, typename Traits>
Resource<ResourceType, Traits> &ResourceManager_base::getResource(const std::string &resname) {
    Resource_base *res = findResource(resname);
    if (res == 0) {
        throw ResourceException("Could not find resource \"" +
                                resname + "\"");
    }

    Resource<ResourceType, Traits> *res_type =
        dynamic_cast<Resource<ResourceType, Traits> *>(res);
    if (res_type == 0) {
        throw ResourceException("Could not convert resource \"" +
                                resname +
                                "\"");
    }

    return *res_type;
}

typedef Resource<bool, BoolTraits> BoolResource;
typedef Resource<int, IntTraits<int> > IntResource;
typedef Resource<unsigned int, IntTraits<unsigned int> > UIntResource;
typedef Resource<std::string, StringTraits> StringResource;

} // end namespace FbTk

#endif // FBTK_RESOURCE_HH
