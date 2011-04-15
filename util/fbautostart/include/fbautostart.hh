/*
 * fbautostart.hh
 * 
 * This file is part of the Fluxbox Autostart ( fbautostart )
 * Utility.
 *
 * Copyright (C) 2011 by Paul Tagliamonte <paultag@ubuntu.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _FBAUTOSTART_H
#define _FBAUTOSTART_H ohai

#define PACKAGE         "fbautostart"
#define PACKAGE_VERSION "2.71828"

#ifndef _DEBUG_MODE
#define _DEBUG_MODE 0
#endif

extern const char * _ON_BEHALF_OF;
extern bool noexec;

#include "dot_desktop.hh"

#define _DEFAULT_XDG_HOME  "~/.config"
#define _DEFAULT_XDG_DIRS  "/etc/xdg"
#define _XDG_AUTOSTART_DIR "/autostart/"

// XXX: Please document these functions better.
//      the @param names need to be fixed up as well.
//      stuff like `foo' or `loc' sucks ass.
//      -PRT

namespace fbautostart {

	/**
	 * Print a help message to the user
	 */
	void help();

	/**
	 * Lecture the user on correct invocation.
	 */
	void lecture();

	/**
	 * Print version of fbautostart to stdout.
	 */
	void version();

	/**
	 * Log an error to stderr.
	 * @param s std::string with the error message
	 */
	void logError( std::string s );

	/**
	 * Log an error to stderr, to print a single integer.
	 * @param i int to print to stderr
	 */
	void logError( int i );

	/**
	 * Print the string to the screen if debug mode is enabled.
	 * @param s std::string with the message to print to the screen
	 */
	void debug( const std::string & s );

	/**
	 * Print a std::vector to the screen if debug mode is enabled.
	 * @param foo std::vector<std::string> to dump
	 */
	void debug( const std::vector<std::string> & foo );

	/**
	 * Print a size_t to the screen if debug mode is enabled.
	 * @param foo size_t to be printed to the screen
	 */
	void debug( size_t foo );

	/**
	 * processArgs should only be called by main, and pass the main arguments to processArgs.
	 * @param argc number of arguments in args
	 * @param argv char array of arguments
	 */
	void processArgs( int argc, char ** args );

	/**
	 * Run a command on the system
	 * @param appl command to run
	 * @return return status of the application
	 */
	int runCommand( std::string appl );

	/**
	 * Break a line up based on a ":"
	 * @param locs the std::vector to push tokens back into
	 * @param lines to process
	 */
	void breakupLine( std::vector<std::string> * locs, std::string lines );

	/**
	 * Fix paths with a tilde in them.
	 * @param locs a std::vector of paths to fix up
	 * @param home the home path of the user, absolute
	 */
	void fixHomePathing( std::vector<std::string> * locs, std::string home );

	/**
	 * Get configuration directories according to the XDG spec.
	 * @param loc std::vector to load up results in
	 * @return returns false if we are unable to complete processing (such as null $HOME)
	 */
	bool getConfDirs( std::vector<std::string> & loc );

	/**
	 * Get all the .desktop files to process.
	 * @param dirs the directories to search
	 * @param out_files the vector we are loading up
	 * @return if we were able to complete processing with success
	 */
	bool getDesktopFiles(
		const std::vector<std::string> & dirs,
		std::vector<dot_desktop>       & out_files
	);
}

#endif
