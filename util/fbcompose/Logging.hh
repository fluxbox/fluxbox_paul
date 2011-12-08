/** Logging.hh file for the fluxbox compositor. */

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


#ifndef FBCOMPOSITOR_LOGGING_HH
#define FBCOMPOSITOR_LOGGING_HH

#include <iostream>


namespace FbCompositor {

    /** Logging level 0. No messages will be printed. */
    const int LOG_LEVEL_NONE = 0;

    /** Logging level 1. Only the error messages will be printed. */
    const int LOG_LEVEL_ERROR = 1;

    /** Logging level 2. Level 1 and warning messages will be printed. */
    const int LOG_LEVEL_WARN = 2;

    /** Logging level 3. Level 2 and information messages will be printed. */
    const int LOG_LEVEL_INFO = 3;

    /** Logging level 4. Level 3 and debug messages will be printed. */
    const int LOG_LEVEL_DEBUG = 4;

    /** Logging level 5. Level 4 and dump messages will be printed. */
    const int LOG_LEVEL_DEBUG_DUMP = 5;


    /**
     * The log manager class.
     */
    class Logger {
    public :
        //--- MAIN METHODS -----------------------------------------------------

        /** \returns the current logging level. */
        static int loggingLevel();

        /** Sets a new logging level. */
        static void setLoggingLevel(int new_level);


    private :
        //--- CONSTRUCTORS -----------------------------------------------------

        /** Default constructor. */
        Logger();

        /** Copy constructor. */
        Logger(const Logger&);

        /** Assignment operator. */
        Logger &operator=(const Logger&);


        //--- PRIVATE VARIABLES ------------------------------------------------

        /** The logging level. */
        static int m_level;
    };

}


#define fbLog_error fbLog_internal(FbCompositor::LOG_LEVEL_ERROR, "[Error] ")
#define fbLog_warn fbLog_internal(FbCompositor::LOG_LEVEL_WARN, "[Warn] ")
#define fbLog_info fbLog_internal(FbCompositor::LOG_LEVEL_INFO, "[Info] ")

#ifdef DEBUG
    #define fbLog_debug fbLog_internal(FbCompositor::LOG_LEVEL_DEBUG, "[Debug] ")
    #define fbLog_debugDump fbLog_internal(FbCompositor::LOG_LEVEL_DEBUG_DUMP, "[Dump] ")
#else
    #define fbLog_debug if (0) std::cerr
    #define fbLog_debugDump if (0) std::cerr
#endif  // DEBUG

// Do not call directly!
#define fbLog_internal(minLevel, levelName) if (FbCompositor::Logger::loggingLevel() >= (minLevel)) std::cerr << (levelName)


#endif  // FBCOMPOSITOR_LOGGING_HH
