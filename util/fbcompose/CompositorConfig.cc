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


#include "CompositorConfig.hh"

#include "Constants.hh"
#include "Logging.hh"

#ifdef USE_XRENDER_COMPOSITING
#include <X11/extensions/Xrender.h>
#endif  // USE_XRENDER_COMPOSITING

#include <algorithm>
#include <iostream>
#include <sstream>

#include <cstdlib>

using namespace FbCompositor;


//--- CONSTRUCTORS AND DESTRUCTORS ---------------------------------------------

// Constructor.
CompositorConfig::CompositorConfig(std::vector<FbTk::FbString> args) :
    m_args(args),

#ifdef USE_OPENGL_COMPOSITING
    m_rendering_mode(RM_OpenGL),
#else
#ifdef USE_XRENDER_COMPOSITING
    m_rendering_mode(RM_XRender),
#else
    m_rendering_mode(RM_ServerAuto),
#endif  // USE_XRENDER_COMPOSITING
#endif  // USE_OPENGL_COMPOSITING

#ifdef USE_XRENDER_COMPOSITING
    m_xrender_pict_filter(FilterFast),
#endif  // USE_XRENDER_COMPOSITING

    m_display_name(""),
    m_frames_per_second(60),
    m_plugins(),
    m_show_x_errors(true),
    m_synchronize(false),
    m_user_plugin_dir("~/.fluxbox/fbcompose/plugins") {

    preScanArguments();
    processArguments();
}

// Copy constructor.
CompositorConfig::CompositorConfig(const CompositorConfig &other) :
    m_args(other.m_args),

    m_rendering_mode(other.m_rendering_mode),
#ifdef USE_XRENDER_COMPOSITING
    m_xrender_pict_filter(other.m_xrender_pict_filter),
#endif  // USE_XRENDER_COMPOSITING

    m_display_name(other.m_display_name),
    m_frames_per_second(other.m_frames_per_second),
    m_plugins(other.m_plugins),
    m_show_x_errors(other.m_show_x_errors),
    m_synchronize(other.m_synchronize),
    m_user_plugin_dir(other.m_user_plugin_dir) {
}

// Assignment operator.
CompositorConfig &CompositorConfig::operator=(const CompositorConfig &other) {
    if (this != &other) {
        m_args = other.m_args;

        m_rendering_mode = other.m_rendering_mode;
#ifdef USE_XRENDER_COMPOSITING
        m_xrender_pict_filter = other.m_xrender_pict_filter;
#endif  // USE_XRENDER_COMPOSITING

        m_display_name = other.m_display_name;
        m_frames_per_second = other.m_frames_per_second;
        m_plugins = other.m_plugins;
        m_show_x_errors = other.m_show_x_errors;
        m_synchronize = other.m_synchronize;
        m_user_plugin_dir = other.m_user_plugin_dir;
    }
    return *this;
}

// Destructor.
CompositorConfig::~CompositorConfig() { }


//--- INTERNAL FUNCTIONS -------------------------------------------------------

// Make the first scan of the arguments for special options.
void CompositorConfig::preScanArguments() {
    std::vector<FbTk::FbString>::iterator it = m_args.begin();

    while (it != m_args.end()) {
        if ((*it == "-h") || (*it == "--help")) {
            printFullHelp(std::cout);
            exit(EXIT_SUCCESS);
        } else if ((*it == "-V") || (*it == "--version")) {
            printVersion(std::cout);
            exit(EXIT_SUCCESS);
        }
        ++it;
    }
}

// Properly scan the command line arguments.
void CompositorConfig::processArguments() {
    std::vector<FbTk::FbString>::iterator it = m_args.begin();
    std::stringstream ss;

    bool be_quiet = false;
    int logging_level = 0;

    while (it != m_args.end()) {
        if ((*it == "-d") || (*it == "--display")) {
            m_display_name = getNextOption(it, "No display string specified.");

        } else if ((*it == "-m") || (*it == "--mode")) {
            FbTk::FbString mode = getNextOption(it, "No rendering mode specified.");

#ifdef USE_OPENGL_COMPOSITING
            if (mode == "opengl") {
                m_rendering_mode = RM_OpenGL;
            } else
#endif  // USE_OPENGL_COMPOSITING
                
#ifdef USE_XRENDER_COMPOSITING
            if (mode == "xrender") {
                m_rendering_mode = RM_XRender;
            } else
#endif  // USE_XRENDER_COMPOSITING

            if (mode == "serverauto") {
                m_rendering_mode = RM_ServerAuto;
            } else {
                ss.str("");
                ss << "Unknown rendering mode \"" << mode << "\".";
                throw ConfigException(ss.str());
            }

        } else if (*it == "--no-x-errors") {
            m_show_x_errors = false;

        } else if ((*it == "-p") || (*it == "--plugin")) {
            FbTk::FbString plugin_name = getNextOption(it, "No plugin name specified.");
            m_plugins.push_back(make_pair(plugin_name, std::vector<FbTk::FbString>()));

        } else if ((*it == "-q") || (*it == "--quiet")) {
            be_quiet = true;

        } else if ((*it == "-r") || (*it == "--refresh-rate")) {
            ss.str(getNextOption(it, "No refresh rate specified."));
            ss >> m_frames_per_second;

            if (m_frames_per_second <= 0) {
                ss.str("");
                ss << "Invalid refresh rate given.";
                throw ConfigException(ss.str());
            }

        } else if (*it == "--sync") {
            m_synchronize = true;

        } else if ((*it == "-v") || (*it == "--verbose")) {
            logging_level += 1;

        } else if (*it == "-vv") {
            logging_level += 2;

        } else if (*it == "-vvv") {
            logging_level += 3;

        } else {
            ss.str("");
            ss << "Unknown option \"" << *it << "\".";
            throw ConfigException(ss.str());
        }
        ++it;
    }

    if (be_quiet) {
        Logger::setLoggingLevel(LOG_LEVEL_NONE);
    } else {
        if (logging_level == 0) {
            Logger::setLoggingLevel(LOG_LEVEL_WARN);
        } else if (logging_level == 1) {
            Logger::setLoggingLevel(LOG_LEVEL_INFO);
        } else if (logging_level == 2) {
            Logger::setLoggingLevel(LOG_LEVEL_DEBUG);
        } else if (logging_level >= 3) {
            Logger::setLoggingLevel(LOG_LEVEL_DEBUG_DUMP);
        }
    }
}


// Fetch the value of the next command line argument, advance iterator.
FbTk::FbString CompositorConfig::getNextOption(std::vector<FbTk::FbString>::iterator &it, const char *error_message) {
    ++it;
    if (it == m_args.end()) {
        throw ConfigException(error_message);
    }
    return *it;
}


//--- CONVENIENCE FUNCTIONS ----------------------------------------------------

// Output full help message.
void CompositorConfig::printFullHelp(std::ostream &os) {
    static const char modes[] = 
#ifdef USE_OPENGL_COMPOSITING
        "opengl, "
#endif  // USE_OPENGL_COMPOSITING
#ifdef USE_XRENDER_COMPOSITING
        "xrender, "
#endif  // USE_XRENDER_COMPOSITING
        "serverauto";

    // 80 character reference (final column = 92)
    //     ................................................................................
    os << "Usage: fbcompose [OPTION] ..." << std::endl
       << "Options and arguments:" << std::endl
       << "  -d <display>, --display <display>" << std::endl
       << "                    Use the specified display connection." << std::endl
       << "  -h, --help        Print this text and exit." << std::endl
       << "  -m <mode>, --mode <mode>" << std::endl
       << "                    Select the rendering mode." << std::endl
       << "                    <mode> can be " << modes << "." << std::endl
       << "  --no-x-errors     Do not print X errors." << std::endl
       << "  -p <plugin>, --plugin <plugin>" << std::endl
       << "                    Load a specified plugin. Run fbcompose-list_plugins script" << std::endl
       << "                    to see all available plugins." << std::endl
       << "  -q, --quiet       Do not print anything." << std::endl
       << "  -r <rate>, --refresh-rate <rate>" << std::endl
       << "                    Specify the compositor's refresh rate in Hz." << std::endl
       << "  --sync            Synchronize with the X server (useful for debugging)." << std::endl
       << "  -v, --verbose     Print more information. Pass several times for more output." << std::endl
       << "  -V, --version     Print version and exit." << std::endl;
}

// Output short help message.
void CompositorConfig::printShortHelp(std::ostream &os) {
    os << "Usage: fbcompose [OPTION]..." << std::endl
       << "Try `fbcompose --help` for more information." << std::endl;
}

// Output version information.
void CompositorConfig::printVersion(std::ostream &os) {
    os << "Fluxbox compositor " << APP_VERSION << std::endl
       << "Copyright (c) 2011 Gediminas Liktaras" << std::endl;
}
