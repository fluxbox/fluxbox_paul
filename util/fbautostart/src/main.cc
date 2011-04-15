/*
 * main.cc
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

#include <iostream>

#include "fbautostart.hh"

using namespace std;

int main ( int argc, char ** argv ) {
	fbautostart::processArgs( argc, argv );

	if ( noexec ) {
		std::cout << "Warning: In noexec mode." << std::endl;
	}

	std::cout << "Launching on behalf of " << _ON_BEHALF_OF << std::endl;

	std::vector<fbautostart::dot_desktop>   files;
	std::vector<std::string>   dirs;

	if ( fbautostart::getConfDirs( dirs ) ) { // if no directories barf in our face
		if ( fbautostart::getDesktopFiles( dirs, files ) ) { // and we load everything with glee
			for ( unsigned int i = 0; i < files.size(); ++i ) { // run through all the files
				fbautostart::dot_desktop d = files.at(i);
				bool happy = true;
				std::string only = d.getAttr("OnlyShowIn"); // Only one per file ( per xdg )
				std::string noti = d.getAttr("NotShowIn");  // We'll ignore that until we care
				                                            // XXX: This is most likely a bug.
				if ( only != "" ) { // even if an attr does not exist
				                    // the object will return it as "".
					int index = -1;
					index = only.find(_ON_BEHALF_OF); // if we have our WM in the OnlyLaunch
					if ( index < 0 ) { // we're disabled ( not found )
						happy = false;
						fbautostart::debug("");
						fbautostart::debug("Not running the following app ( Excluded by a OnlyShowIn )");
						fbautostart::debug(d.getAttr("Name"));
					}
				}
				if ( noti != "" ) { // (NotShowIn (don't show))
					int index = -1;
					index = noti.find(_ON_BEHALF_OF); // if we have found our WM
					if ( index >= 0 ) { // We're in Launch, stop from launching it.
						happy = false;
						fbautostart::debug("");
						fbautostart::debug("Forced into not running the following app ( Included by not being in NotShowIn )");
						fbautostart::debug(d.getAttr("Name"));
					}
				}
				if ( d.getAttr("Hidden") == "" && happy ) { // If we sould exec
					std::string appl = d.getAttr("Exec"); // get the line to run
					if ( appl != "" ) { // if it's defined and ready to go
						fbautostart::debug( "Processing File: ");
						fbautostart::debug(d.getFile());
						fbautostart::runCommand( appl ); // kickoff (regardless of noexec)
					}
				} // otherwise, we're out of here.
			}
			return 0;
		}
		return 0xDEADBEEF;
	}
	return 0xCAFEBABE;
}
