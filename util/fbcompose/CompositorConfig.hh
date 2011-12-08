/** CompositorConfig.cc file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_COMPOSITORCONFIG_HH
#define FBCOMPOSITOR_COMPOSITORCONFIG_HH

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif  // HAVE_CONFIG_H

#include "Enumerations.hh"
#include "Exceptions.hh"

#include "FbTk/FbString.hh"

#include <X11/Xlib.h>

#include <vector>
#include <iosfwd>


namespace FbCompositor {

    /**
     * Handles the compositor's configuration.
     *
     * This class is responsible for obtaining the compositor's configuration,
     * ensuring that all of it is correct and presenting it to the Compositor
     * class.
     */
    class CompositorConfig {
    public:
        //--- CONSTRUCTORS AND DESTRUCTORS -------------------------------------

        /** Constructor. */
        CompositorConfig(std::vector<FbTk::FbString> args);

        /** Copy constructor. */
        CompositorConfig(const CompositorConfig &other);

        /** Assignment operator. */
        CompositorConfig &operator=(const CompositorConfig &other);

        /** Destructor. */
        ~CompositorConfig();


        //--- ACCESSORS --------------------------------------------------------

        /** \returns the display name. */
        const FbTk::FbString &displayName() const;

        /** \returns the refresh rate. */
        int framesPerSecond() const;

        /** \returns the selected rendering mode. */
        RenderingMode renderingMode() const;

        /** \returns whether the X errors should be printed. */
        bool showXErrors() const;

        /** \returns whether the compositor should synchronize with the X server. */
        bool synchronize() const;

        /** \returns the user plugin directory. */
        const FbTk::FbString &userPluginDir() const;

#ifdef USE_XRENDER_COMPOSITING
        /** \returns the XRender picture filter. */
        const char *xRenderPictFilter() const;
#endif  // USE_XRENDER_COMPOSITING


        /** \returns the number of available plugins. */
        int pluginCount() const;

        /** \returns the name of the given plugin. */
        const FbTk::FbString &pluginName(int plugin_id) const;

        /** \returns the arguments to the given plugin. */
        const std::vector<FbTk::FbString> &pluginArgs(int plugin_id) const;


        //--- CONVENIENCE FUNCTIONS --------------------------------------------

        /** Output full help message. */
        static void printFullHelp(std::ostream &os);

        /** Output short help message. */
        static void printShortHelp(std::ostream &os);

        /** Output version information. */
        static void printVersion(std::ostream &os);

    private:
        //--- INTERNAL FUNCTIONS -----------------------------------------------

        /** Make the first scan of the arguments for special options. */
        void preScanArguments();

        /** Properly scan the command line arguments. */
        void processArguments();


        /** Fetch the value of the next command line argument, advance iterator. */
        FbTk::FbString getNextOption(std::vector<FbTk::FbString>::iterator &it, const char *error_message);


        //--- PRIVATE VARIABLES ------------------------------------------------

        /** The passed command line arguments. */
        std::vector<FbTk::FbString> m_args;


        /** Selected rendering mode. */
        RenderingMode m_rendering_mode;

#ifdef USE_XRENDER_COMPOSITING
        /** XRender picture filter. */
        FbTk::FbString m_xrender_pict_filter;
#endif  // USE_XRENDER_COMPOSITING


        /** The name of the display we want to use. */
        FbTk::FbString m_display_name;

        /** The refresh rate. */
        int m_frames_per_second;

        /** Plugins and their arguments. */
        std::vector< std::pair< FbTk::FbString, std::vector<FbTk::FbString> > > m_plugins;

        /** Whether the X errors should be printed. */
        bool m_show_x_errors;

        /** Whether the compositor should synchronize with the X server. */
        bool m_synchronize;

        /** User plugin directory. */
        FbTk::FbString m_user_plugin_dir;
    };


    //--- INLINE FUNCTIONS -----------------------------------------------------

    // Returns the display name.
    inline const FbTk::FbString &CompositorConfig::displayName() const {
        return m_display_name;
    }

    // Returns the refresh rate.
    inline int CompositorConfig::framesPerSecond() const {
        return m_frames_per_second;
    }

    // Returns the arguments to the given plugin.
    inline const std::vector<FbTk::FbString> &CompositorConfig::pluginArgs(int plugin_id) const {
        if ((plugin_id < 0) || (plugin_id >= pluginCount())) {
            throw IndexException("Out of bounds index in CompositorConfig::pluginArgs.");
        }
        return m_plugins[plugin_id].second;
    }

    // Returns the number of available plugins.
    inline int CompositorConfig::pluginCount() const {
        return (int)(m_plugins.size());
    }

    // Returns the name of the given plugin.
    inline const FbTk::FbString &CompositorConfig::pluginName(int plugin_id) const {
        if ((plugin_id < 0) || (plugin_id >= pluginCount())) {
            throw IndexException("Out of bounds index in CompositorConfig::pluginName.");
        }
        return m_plugins[plugin_id].first;
    }

    // Returns the rendering mode.
    inline RenderingMode CompositorConfig::renderingMode() const {
        return m_rendering_mode;
    }

    // Returns whether the X errors should be printed.
    inline bool CompositorConfig::showXErrors() const {
        return m_show_x_errors;
    }

    // Returns whether the compositor should synchronize with the X server.
    inline bool CompositorConfig::synchronize() const {
        return m_synchronize;
    }

    // Returns the user plugin directory.
    inline const FbTk::FbString &CompositorConfig::userPluginDir() const {
        return m_user_plugin_dir;
    }

#ifdef USE_XRENDER_COMPOSITING
    // Returns the XRender picture filter.
    inline const char *CompositorConfig::xRenderPictFilter() const {
        return m_xrender_pict_filter.c_str();
    }
#endif  // USE_XRENDER_COMPOSITING
}

#endif  // FBCOMPOSITOR_COMPOSITORCONFIG_HH
