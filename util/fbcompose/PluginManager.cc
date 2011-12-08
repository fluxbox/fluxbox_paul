/** PluginManager.cc file for the fluxbox compositor. */

// Copyright (c) 2011 Gediminas Liktaras (gliktaras at gmail dot com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif  // HAVE_CONFIG_H

#include "PluginManager.hh"

#include "BasePlugin.hh"
#include "BaseScreen.hh"
#include "Exceptions.hh"
#include "Logging.hh"
#include "Utility.hh"

#include <algorithm>
#include <dlfcn.h>
#include <sstream>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Costructor.
PluginManager::PluginManager(PluginType plugin_type, const BaseScreen &screen,
                             const FbTk::FbString user_plugin_dir) :
    m_screen(screen) {

    m_plugin_type = plugin_type;
    m_user_plugin_dir = user_plugin_dir;
}

// Destructor.
PluginManager::~PluginManager() {
    for (size_t i = 0; i < m_plugin_objects.size(); i++) {
        delete m_plugin_objects[i];
    }

    std::map<FbTk::FbString, PluginLibData>::iterator it = m_plugin_libs.begin();
    while (it != m_plugin_libs.end()) {
        unloadPlugin(it);
        ++it;
    }
}


//--- PLUGIN MANIPULATION ------------------------------------------------------

// Create a plugin object, load the appropriate library if needed.
void PluginManager::createPluginObject(FbTk::FbString name, std::vector<FbTk::FbString> args) {
    if (m_plugin_libs.find(name) == m_plugin_libs.end()) {
        loadPlugin(name);
    }

    CreatePluginFunction create_function = m_plugin_libs.find(name)->second.create_function;
    BasePlugin *new_plugin_object = (*create_function)(m_screen, args);
    m_plugin_objects.push_back(new_plugin_object);
}


//--- INTERNAL PLUGIN MANIPULATION ---------------------------------------------

// Load a plugin.
void PluginManager::loadPlugin(FbTk::FbString name) {
    static union {
        void *void_ptr;
        PluginTypeFunction plugin_type_func;
        CreatePluginFunction create_plugin_func;
    } obj_union;

    std::vector<FbTk::FbString> paths = buildPluginPaths(name);

    // Get the handle to the plugin so object.
    void *handle = NULL;
    for (size_t i = 0; i < paths.size(); i++) {
        handle = dlopen(paths[i].c_str(), RTLD_LAZY | RTLD_LOCAL);
        if (handle) {
            break;
        }
    }
    if (!handle) {
        std::stringstream ss;
        ss << "Could not find/load plugin \"" << name << "\".";
        throw PluginException(ss.str());
    }

    // Check for the correct plugin type.
    obj_union.void_ptr = getLibraryObject(handle, "pluginType", name.c_str(), "type function");
    PluginTypeFunction type_func = obj_union.plugin_type_func;

    if ((*(type_func))() != m_plugin_type) {
        std::stringstream ss;
        ss << "Plugin \"" << name << "\" is of the wrong type.";
        throw PluginException(ss.str());
    }

    // Get the plugin creation function.
    obj_union.void_ptr = getLibraryObject(handle, "createPlugin", name.c_str(), "creation function");
    CreatePluginFunction create_func = obj_union.create_plugin_func;

    // Track the loaded plugin.
    PluginLibData plugin_data = { handle, create_func };
    m_plugin_libs.insert(make_pair(name, plugin_data));
}

// Unload a plugin.
void PluginManager::unloadPlugin(FbTk::FbString name) {
    std::map<FbTk::FbString, PluginLibData>::iterator it = m_plugin_libs.find(name);

    if (it == m_plugin_libs.end()) {
        std::stringstream ss;
        ss << "Plugin \"" << name << "\" is not loaded (unloadPlugin).";
        throw PluginException(ss.str());
    } else {
        unloadPlugin(it);
    }
}

// Unload a plugin (actual worker function).
void PluginManager::unloadPlugin(std::map<FbTk::FbString, PluginLibData>::iterator it) {
    dlclose(it->second.handle);

    it->second.handle = NULL;
    it->second.create_function = NULL;
}


//--- CONVENIENCE FUNCTIONS ----------------------------------------------------

// Build a vector of search paths for a given plugin.
std::vector<FbTk::FbString> PluginManager::buildPluginPaths(const FbTk::FbString &name) {
    std::stringstream ss;
    std::vector<FbTk::FbString> paths;

    FbTk::FbString type_dir = "";
    if (m_plugin_type == Plugin_OpenGL) {
        type_dir = "opengl/";
    } else if (m_plugin_type == Plugin_XRender) {
        type_dir = "xrender/";
    }

    ss << "./plugins/" << type_dir << name << "/.libs/" << name << ".so";
    paths.push_back(ss.str());
    ss.str("");

    ss << m_user_plugin_dir << type_dir << name << ".so";
    paths.push_back(ss.str());
    ss.str("");

#ifdef FBCOMPOSE_PATH
    ss << FBCOMPOSE_PATH << "/plugins/" << type_dir << name << ".so";
    paths.push_back(ss.str());
    ss.str("");
#endif

    paths.push_back(name);

    return paths;
}

// Returns some object from the given library handle.
void *PluginManager::getLibraryObject(void *handle, const char *object_name, const char *plugin_name,
                                      const char *verbose_object_name) {
    dlerror();
    void *raw_object = dlsym(handle, object_name);
    const char *error = dlerror();

    if (error) {
        dlclose(handle);
        std::stringstream ss;
        ss << "Error in loading " << verbose_object_name << " for \"" << plugin_name << "\" plugin: " << error;
        throw PluginException(ss.str());
    } else {
        return raw_object;
    }
}
