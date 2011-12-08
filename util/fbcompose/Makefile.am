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

SUBDIRS=		plugins

AM_CXXFLAGS=		-Wall -Wextra -pedantic
FBCOMPOSE_PATH=		@FBCOMPOSE_PATH@
INCLUDES=		-I$(top_srcdir)/src -I$(top_srcdir)/src/FbTk

bin_PROGRAMS=		fbcompose

if SERVERAUTO_COMPOSITING_ONLY
fbcompose_main_SOURCES=
else
fbcompose_main_SOURCES=		BaseCompWindow.hh BaseCompWindow.cc \
				BasePlugin.hh BasePlugin.cc \
				BaseScreen.hh BaseScreen.cc \
				Compositor.hh Compositor.cc \
				PluginManager.hh PluginManager.cc \
				TickTracker.hh TickTracker.cc \
				Utility.hh Utility.cc
endif	# SERVERAUTO_COMPOSITING_ONLY

if OPENGL_COMPOSITING
fbcompose_opengl_SOURCES=	OpenGLPlugin.hh OpenGLPlugin.cc \
				OpenGLResources.hh OpenGLResources.cc \
				OpenGLScreen.hh OpenGLScreen.cc \
				OpenGLShaders.hh OpenGLShaders.cc \
				OpenGLTexPartitioner.hh OpenGLTexPartitioner.cc \
				OpenGLUtility.hh OpenGLUtility.cc \
				OpenGLWindow.hh OpenGLWindow.cc
else
fbcompose_opengl_SOURCES=
endif	# OPENGL_COMPOSITING

if XRENDER_COMPOSITING
fbcompose_xrender_SOURCES=	XRenderPlugin.hh XRenderPlugin.cc \
				XRenderResources.hh XRenderResources.cc \
				XRenderScreen.hh XRenderScreen.cc \
				XRenderWindow.hh XRenderWindow.cc
else
fbcompose_xrender_SOURCES=
endif	# XRENDER_COMPOSITING

fbcompose_SOURCES=	main.cc \
			Atoms.hh Atoms.cc \
			CompositorConfig.hh CompositorConfig.cc \
			Constants.hh \
			Enumerations.hh \
			Exceptions.hh \
			Logging.hh Logging.cc \
			ServerAutoApp.hh ServerAutoApp.cc \
			${fbcompose_main_SOURCES} \
                        ${fbcompose_opengl_SOURCES} \
                        ${fbcompose_xrender_SOURCES}

fbcompose_LDADD=	$(top_builddir)/src/FbTk/libFbTk.a


install-exec-hook:
	$(INSTALL_DATA) "$(srcdir)/fbcompose-list_plugins" "$(DESTDIR)$(bindir)"
	sed -i -e "s|[$$]FBCOMPOSE_PATH|${FBCOMPOSE_PATH}|g" "$(DESTDIR)$(bindir)/fbcompose-list_plugins"
	chmod +x "$(DESTDIR)$(bindir)/fbcompose-list_plugins"

$(top_builddir)/src/FbTk/libFbTk.a:
	cd $(top_builddir)/src/FbTk && make
