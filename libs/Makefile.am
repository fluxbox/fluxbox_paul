# lib/Makefile.am for Fluxbox - an X11 Window manager

all-local:
	$(MAKE) -C lua/src VPATH="../../$(srcdir)/lua/src" CC=$(CXX) CFLAGS='$(CXXFLAGS) $$(MYCFLAGS)' linux

clean-local:
	$(MAKE) -C lua/src clean
