/*
 * fbautostart.cc
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

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <iostream>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <string.h>
#include <string>
#include <stdio.h>

#include <sstream>

#include "fbautostart.hh"

#ifndef _FBAUTOSTART_CPP
#define _FBAUTOSTART_CPP foomanchu

#define _APPNAME_ PACKAGE
#define _VERSION_ PACKAGE_VERSION

#ifndef _ON_BEHALF_OF_DEFAULT

/**
 *  I'm a bit biased on what this should
 *  be starting up for. Considering the
 *  name of this app and such.
 */

#define _ON_BEHALF_OF_DEFAULT "FLUXBOX"
#endif

const char * _ON_BEHALF_OF = _ON_BEHALF_OF_DEFAULT;

bool noexec = false;

void fbautostart::version() {
	std::cout << "" << _APPNAME_ << " " << _VERSION_ << std::endl;
}

void fbautostart::lecture() {
	version();
	std::cout << "Copyright (C) 2011 The Fluxbox Team" << std::endl;
	std::cout << "Copyright (C) 2010 Paul Tagliamonte" << std::endl;
	std::cout << "This software is licensed under the MIT License." << std::endl;
	std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
	std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
}

void fbautostart::help() {
	std::cout << "Usage: fbautostart [OPTION] ... " << std::endl;
	std::cout << "Startup all applications according to the" << std::endl;
	std::cout << "XDG Spec. Right now, I'm hardcoded to think that" << std::endl;
	std::cout << "I am doing this on behalf of '" << _ON_BEHALF_OF << "'" << std::endl;

	std::cout << std::endl;

	std::cout << "--license             Print out license information" << std::endl;
	std::cout << "--help                Print out this message" << std::endl;
	std::cout << "--noexec              Don't exec, just do a dry run" << std::endl;
	std::cout << "--window-manager ID   Override hardcoded value ( " << _ON_BEHALF_OF <<  " )" << std::endl;

	std::cout << "" << std::endl;

	std::cout << "Copyleft (c) The Fluxbox Team, 2011, MIT License" << std::endl;
	std::cout << "Copyleft (c) Paul Tagliamonte, 2010, MIT License" << std::endl;
	//                         ^^^^^^^^^^^^^^^^ That is pretty fucking awesome
	//                                          Future maintainer: Your name must
	//                                          be exactly 16 chars long. kthx.

	std::cout << "" << std::endl;
}

int fbautostart::runCommand( std::string appl ) {
	/*
	 * The following is almost directly ripped off from
	 * Fluxbox's implementation. ( at ak|ra's behest )
	 */

	pid_t pid = fork();
	if (pid)
		return pid;

	// get shell path from the environment
	//
	// this process exits immediately, so we don't have to
	//  worry about memleaks
	std::string shell = getenv("SHELL");

	if ( shell != "" )
		shell = "/bin/sh";

	if ( ! noexec ) { // we'll do it live
		execl(
			shell.c_str(),
			shell.c_str(),
			"-c",
			appl.c_str(),
			static_cast<void*>(NULL)
		);
		exit ( EXIT_SUCCESS );
		return pid; // compiler happy -> we are happy ;)
	} else { // dummy mode ( test )
		std::cout << "Would have run: " << appl << std::endl;
		exit(0);
		return 0;
	}
}

void fbautostart::processArgs( int argc, char ** args ) {
	for ( int i = 1; i < argc; ++i ) {
		if ( strcmp(args[i], "--license") == 0 ) {
			lecture();
			exit(0);
		} else if ( strcmp(args[i], "--version") == 0 ) {
			version();
			exit(0);
		} else if ( strcmp(args[i], "--help") == 0 ) {
			help();
			exit(0);
		} else if ( strcmp(args[i], "--noexec") == 0 ) {
			noexec = true;
		} else if ( strcmp(args[i], "--window-manager") == 0 ) {
			_ON_BEHALF_OF = args[i+1];
			i = i+1;
		} else {
			help();

			std::cout << "Holy god. I have no idea what:" << std::endl;
			std::cout << "  " << args[i] << std::endl;
			std::cout << "means. Try again :/" << std::endl;
			std::cout << std::endl;

			exit(1);
		}
	}
}

/*
 * OK. Here's some Spec text. I accessed this data on
 * October 14th, 2010. 
 *
 * The URL was:
 * http://standards.freedesktop.org/autostart-spec/autostart-spec-latest.html
 *
 *
 * == Autostart Directories ==
 *
 * The Autostart Directories are $XDG_CONFIG_DIRS/autostart as
 * defined in accordance with the "Referencing this specification"
 * section in the "desktop base directory specification".
 *
 * If the same filename is located under multiple Autostart Directories
 * only the file under the most important directory should be used.
 *
 * Example: If $XDG_CONFIG_HOME is not set the Autostart Directory
 *          in the user's home directory is ~/.config/autostart/
 *
 * Example: If $XDG_CONFIG_DIRS is not set the system wide Autostart
 *          Directory is /etc/xdg/autostart/
 *
 * Example: If $XDG_CONFIG_HOME and $XDG_CONFIG_DIRS are not set and
 *          the two files /etc/xdg/autostart/foo.desktop and
 *          ~/.config/autostart/foo.desktop exist then only the file
 *          ~/.config/autostart/foo.desktop will be used because
 *          ~/.config/autostart/ is more important than /etc/xdg/autostart/
 */


void fbautostart::logError( std::string s ) { // [e]: my error line
	std::cerr << "[e]: " << s << std::endl;
}

void fbautostart::logError( int i ) { // [e]: (int) n
	std::cerr << "[e]: (int) " << i << std::endl;
}

void fbautostart::debug( const std::string & s ) {    // [l]: my debug line
	if ( _DEBUG_MODE ) {
		std::cout << "[l]: " << s << std::endl;
	}
}

void fbautostart::debug( const std::vector<std::string> & foo ) {    // [l]: my debug line
	if ( _DEBUG_MODE ) {
		std::cout << "[l]: The following is an array" << std::endl;
		for ( unsigned int i = 0; i < foo.size(); ++i ) {
			std::cout << "[l]:   " << foo.at(i) << std::endl;
		}
	}
}

void fbautostart::debug( size_t foo ) {    // [l]: my debug line
	if ( _DEBUG_MODE ) {
		std::cout << "[l]: " << foo << std::endl;
	}
}

void fbautostart::breakupLine( std::vector<std::string> * locs, std::string lines ) {
	std::string token;
	std::istringstream iss(lines);
	while ( getline(iss, token, ':') ) {
		token.append( _XDG_AUTOSTART_DIR ); // use the xdg autostart dir.
		locs->push_back( token );
	}
}

void fbautostart::fixHomePathing( std::vector<std::string> * locs, std::string home ) {
	for ( unsigned int i = 0; i < locs->size(); ++i ) {
		if ( locs->at(i)[0] == '~' && locs->at(i)[1] == '/' ) {
			debug("");
			debug("Fixing a path old / new path follows.");
			debug(locs->at(i));
			locs->at(i).replace(0, 1, home );
			debug(locs->at(i));
		}
	}
}

bool fbautostart::getConfDirs( std::vector<std::string> & loc ) {

	const char * xdg_home = getenv("XDG_CONFIG_HOME"); // See the spec for why
	const char * xdg_dirs = getenv("XDG_CONFIG_DIRS"); // I'm using what I'm using.
	const char * uzr_home = getenv("HOME");

	if ( uzr_home != NULL ) {

		std::string XDG_home, XDG_dirs;

		if ( xdg_dirs != NULL ) {
			debug( "We have an overridden xdg dir. Using it." );
			XDG_dirs = xdg_dirs;
		} else {
			XDG_dirs = _DEFAULT_XDG_DIRS;
		}

		if ( xdg_home != NULL ) {
			debug( "We have an overridden xdg home. Using it." );
			XDG_home = xdg_home;
		} else {
			XDG_home = _DEFAULT_XDG_HOME;
		}

		debug("Following is the targets");
		debug(XDG_home);
		debug(XDG_dirs);

		breakupLine( &loc, XDG_dirs );
		breakupLine( &loc, XDG_home );

		for ( unsigned int i = 0; i < loc.size(); ++i )
			fixHomePathing( &loc, uzr_home );

		debug("");
		debug("Using the following array to find files ( Expanded ): ");
		debug(loc);

		return true;
	} else {
		logError( "the env var HOME is not set. Panic!" );
		return false;
	}
}

bool fbautostart::getDesktopFiles(
	const std::vector<std::string> & dirs,
	std::vector<dot_desktop>       & out_files
) {
	std::vector<dot_desktop> files;

	for ( unsigned int i = 0; i < dirs.size(); ++i ) {
		DIR           * dp   = NULL;
		struct dirent * dirp = NULL;
		if ( (dp = opendir(dirs.at(i).c_str())) == NULL ) {
			if ( errno == ENOENT ) { // E(RROR) NO EXIST ( or whatever )
				debug("");
				debug("Looks like the dir does not exist. Dir follows.");
				debug( dirs.at(i) );
			} else {
				logError("");
				logError("Oh no! Error opening directory! Directory, then Errorno follows: ");
				logError( dirs.at(i) );
				logError(errno);
				return false;
			}
		} else {
			while ((dirp = readdir(dp)) != NULL) { // for every file in the directory
				std::string file(dirp->d_name); // char arrays suck
				if ( file != "." && file != ".."  ) { // make sure we don't use . / ..
					int dupe = -1; // there's no -1st element, silly!

					for ( unsigned int n = 0; n < files.size(); ++n ) {
						if ( files.at(n).getID() == file ) { // make sure it's unique
						                                       // ( as the xdg 
						                                       //    spec requires )

							dupe = n; // there can be only one
							          // dupe in the array, so it's
							          // OK to think that there will
							          // be only one
						}
					}

					std::string desktop_filepath = dirs.at(i);
					desktop_filepath.append(file);
					dot_desktop new_file( desktop_filepath, file );

					if ( dupe >= 0 ) {
						files.at(dupe) = new_file;
					} else {
						files.push_back( new_file );
					}
				} 
			}
			closedir(dp);
		}
	}

	if ( ! files.empty() ) {
		std::swap( files, out_files );
		return true;
	} else {
		return false;
	}
}

#endif
