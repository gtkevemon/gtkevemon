PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

all:
	$(MAKE) -C src

debug:
	$(MAKE) -C src debug

clean:
	$(MAKE) -C src clean

install:
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 src/gtkevemon $(DESTDIR)$(BINDIR)/gtkevemon
	$(MAKE) -C icon

uninstall:
	${RM} $(DESTDIR)$(BINDIR)/gtkevemon
	$(MAKE) -C icon uninstall
