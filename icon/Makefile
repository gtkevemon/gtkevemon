PREFIX ?= /usr/local
DATAROOTDIR ?= $(PREFIX)/share
PIXMAPDIR ?= $(DATAROOTDIR)/pixmaps
DESKTOPDIR ?= $(DATAROOTDIR)/applications

install:
	install -Dm 644 gtkevemon.svg $(DESTDIR)$(PIXMAPDIR)/gtkevemon.svg
	install -Dm 644 gtkevemon.desktop $(DESTDIR)$(DESKTOPDIR)/gtkevemon.desktop

uninstall:
	rm $(DESTDIR)$(PIXMAPDIR)/gtkevemon.svg
	rm $(DESTDIR)$(DESKTOPDIR)/gtkevemon.desktop
