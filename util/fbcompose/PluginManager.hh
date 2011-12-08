/** PluginManager.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_PLUGINMANAGER_HH
#define FBCOMPOSITOR_PLUGINMANAGER_HH

#include "Enumerations.hh"

#include "FbTk/FbString.hh"

#include <map>
#include <vector>


namespace FbCompositor {

    class BasePlugin;
    class BaseScreen;


    //--- TYPEDEFS -------------------------------------------------------------

    /** A pointer to a function that creates a plugin class instance. */
    typedef BasePlugin* (*CreatePluginFunction)(const BaseScreen&, const std::vector<FbTk::FbString>&);

    /** A pointer to a function that returns the rendering mode the plugin operates in. */
    typedef PluginType (*PluginTypeFunction)();


    /**
     * Responsible for plugin loading, unloading and availibility.
     */
    class PluginManager {
        struct PluginLibData;

    public :
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        PluginManager(PluginType plugin_type, const BaseScreen &screen, const FbTk::FbString user_plugin_dir);

        /** Destructor. */
        ~PluginManager();


        //--- PLUGIN MANIPULATION ----------------------------------------------

        /** Create a plugin object, load the appropriate library if needed. */
        void createPluginObject(FbTk::FbString name, std::vector<FbTk::FbString> args = std::vector<FbTk::FbString>());

        /** \returns a reference to a vector with plugin objects. */
        std::vector<BasePlugin*> &plugins();

        /** \returns a reference to a vector with plugin objects (const version). */
        const std::vector<BasePlugin*> &plugins() const;


    private :
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Copy constructor. */
        PluginManager(const PluginManager&);

        /** Assignment operator. */
        PluginManager &operator=(const PluginManager&);


        //--- INTERNAL PLUGIN MANIPULATION -------------------------------------

        /** Load a plugin. */
        void loadPlugin(FbTk::FbString name);

        /** Unload a plugin. */
        void unloadPlugin(FbTk::FbString name);

        /** Unload a plugin (actual worker function). */
        void unloadPlugin(std::map<FbTk::FbString, PluginLibData>::iterator it);


        //--- CONVENIENCE FUNCTIONS --------------------------------------------

        /** Build a vector of search paths for a given plugin. */
        std::vector<FbTk::FbString> buildPluginPaths(const FbTk::FbString &name);

        /** \returns some object from the given library handle. */
        void *getLibraryObject(void *handle, const char *object_name, const char *plugin_name,
                               const char *verbose_object_name);


        //--- PLUGINS AND METADATA ---------------------------------------------

        /** Specific plugin-related data. */
        struct PluginLibData {
            void *handle;                           ///< Handle to the loaded library.
            CreatePluginFunction create_function;   ///< Plugin creation function.
        };

        /** A map, containing all loaded plugins. */
        std::map<FbTk::FbString, PluginLibData> m_plugin_libs;

        /** A vector with active plugin objects. */
        std::vector<BasePlugin*> m_plugin_objects;

        /** Type of the plugins this object manages. */
        PluginType m_plugin_type;

        /** The screen this manager operates on. */
        const BaseScreen &m_screen;

        /** User plugin directory. */
        FbTk::FbString m_user_plugin_dir;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns a reference to a vector with plugin objects.
    inline std::vector<BasePlugin*> &PluginManager::plugins() {
        return m_plugin_objects;
    }

    // Returns a reference to a vector with plugin objects (const version).
    inline const std::vector<BasePlugin*> &PluginManager::plugins() const {
        return m_plugin_objects;
    }
}

#endif  // FBCOMPOSITOR_PLUGINMANAGER_HH
