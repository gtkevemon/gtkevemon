PREFIX ?= /usr/local
DATAROOTDIR ?= $(PREFIX)/share
PIXMAPDIR ?= $(DATAROOTDIR)/pixmaps
DESKTOPDIR ?= $(DATAROOTDIR)/applications

install:
	mkdir -p $(DESTDIR)$(PIXMAPDIR)
	install -m 644 gtkevemon.svg $(DESTDIR)$(PIXMAPDIR)/gtkevemon.svg
	mkdir -p $(DESTDIR)$(DESKTOPDIR)
	install -m 644 gtkevemon.desktop $(DESTDIR)$(DESKTOPDIR)/gtkevemon.desktop

uninstall:
	rm $(DESTDIR)$(PIXMAPDIR)/gtkevemon.svg
	rm $(DESTDIR)$(DESKTOPDIR)/gtkevemon.desktop
