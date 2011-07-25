# data/Makefile.am for Fluxbox - an X11 Window manager

PROGRAM_PREFIX=@program_prefix@
PROGRAM_SUFFIX=@program_suffix@

DEFAULT_MENU = @DEFAULT_MENU@
DEFAULT_STYLE = @DEFAULT_STYLE@
DEFAULT_KEYS = @DEFAULT_KEYS@
DEFAULT_APPS = @DEFAULT_APPS@
DEFAULT_OVERLAY = @DEFAULT_OVERLAY@
DEFAULT_INIT = @DEFAULT_INIT@
DEFAULT_WINDOWMENU = @DEFAULT_WINDOWMENU@

SUBDIRS= 		styles
CLEANFILES= 		menu.lua init.lua apps
MAINTAINERCLEANFILES= 	Makefile.in
pkgdata_DATA= 		keys.lua menu.lua init.lua apps overlay windowmenu.lua
EXTRA_DIST= 		keys.lua init.lua.in apps.in overlay windowmenu.lua
all-local: 		menu.lua init.lua apps
install-pkgdataDATA:
	@echo "Installing menu file in $(DESTDIR)$(DEFAULT_MENU)"
	$(mkinstalldirs) $(DESTDIR)$(dir $(DEFAULT_MENU))
	$(INSTALL_DATA) ./menu.lua $(DESTDIR)$(DEFAULT_MENU)
	@echo "Installing keys file in $(DESTDIR)$(DEFAULT_KEYS)"
	$(mkinstalldirs) $(DESTDIR)$(dir $(DEFAULT_KEYS))
	$(INSTALL_DATA) $(srcdir)/keys.lua $(DESTDIR)$(DEFAULT_KEYS)
	@echo "Installing apps file in $(DESTDIR)$(DEFAULT_APPS)"
	$(mkinstalldirs) $(DESTDIR)$(dir $(DEFAULT_APPS))
	$(INSTALL_DATA) ./apps $(DESTDIR)$(DEFAULT_APPS)
	@echo "Installing overlay file in $(DESTDIR)$(DEFAULT_OVERLAY)"
	$(mkinstalldirs) $(DESTDIR)$(dir $(DEFAULT_OVERLAY))
	$(INSTALL_DATA) $(srcdir)/overlay $(DESTDIR)$(DEFAULT_OVERLAY)
	@echo "Installing init file in $(DESTDIR)$(DEFAULT_INIT)"
	$(mkinstalldirs) $(DESTDIR)$(dir $(DEFAULT_INIT))
	$(INSTALL_DATA) ./init.lua $(DESTDIR)$(DEFAULT_INIT)
	@echo "Installing windowmenu file in $(DESTDIR)$(DEFAULT_WINDOWMENU)"
	$(mkinstalldirs) $(DESTDIR)$(dir $(DEFAULT_WINDOWMENU))
	$(INSTALL_DATA) $(srcdir)/windowmenu.lua $(DESTDIR)$(DEFAULT_WINDOWMENU)

distclean-local:
		rm -f *\~
init.lua: 		init.lua.in
	@regex_cmd@ -e "s,@pkgdatadir@,$(pkgdatadir),g" \
			-e "s,@pkgprefix@,$(PROGRAM_PREFIX:NONE=),g" \
			-e "s,@pkgsuffix@,$(PROGRAM_SUFFIX:NONE=),g" \
			-e "s,@default_style@,$(DEFAULT_STYLE),g" $(srcdir)/init.lua.in > init.lua

apps: 			apps.in
	@regex_cmd@ -e "s,@pkgdatadir@,$(pkgdatadir),g" \
			-e "s,@pkgprefix@,$(PROGRAM_PREFIX:NONE=),g" \
			-e "s,@pkgsuffix@,$(PROGRAM_SUFFIX:NONE=),g" $(srcdir)/apps.in > apps

menu.lua: 		../util/fluxbox-generate_menu
	env	MENUFILENAME=menu.lua \
		PREFIX="${prefix}" \
		MENUTITLE="Fluxbox-${VERSION}" \
		INSTALL=Yes \
		PATH="${PATH}:../util:../util/fbrun" \
		../util/fluxbox-generate_menu

../util/fluxbox-generate_menu:
	cd ../util && ${MAKE} fluxbox-generate_menu
