DATAROOTDIR = $(PREFIX)/share
PIXMAPDIR = $(DATAROOTDIR)/pixmaps
DESKTOPDIR = $(DATAROOTDIR)/applications

install:
	install -Dm 644 gtkevemon.svg $(PIXMAPDIR)/gtkevemon.svg
	install -Dm 644 gtkevemon.desktop $(DESKTOPDIR)/gtkevemon.desktop

uninstall:
	rm $(PIXMAPDIR)/gtkevemon.svg
	rm $(DESKTOPDIR)/gtkevemon.desktop
