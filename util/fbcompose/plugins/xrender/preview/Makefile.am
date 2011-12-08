## Makefile.am file for the fluxbox compositor.

# Copyright (c) 2011 Gediminas Liktaras (gliktaras at gmail dot com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

FBCOMPOSE_PATH = @FBCOMPOSE_PATH@

AM_CXXFLAGS = -Wall -Wextra -pedantic -fPIC
AM_LDFLAGS  = -export-dynamic -shared -module -avoid-version

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/FbTk -I$(top_srcdir)/util/fbcompose


plugindir = $(FBCOMPOSE_PATH)/plugins/xrender
plugin_LTLIBRARIES = preview.la
preview_la_LIBADD  = $(top_builddir)/src/FbTk/libFbTk.a
preview_la_SOURCES = PreviewPlugin.hh PreviewPlugin.cc


$(top_builddir)/src/FbTk/libFbTk.a:
	cd $(top_builddir)/src/FbTk && make
