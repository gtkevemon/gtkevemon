PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

all:
	$(MAKE) -C src

debug:
	$(MAKE) -C src debug

clean:
	$(MAKE) -C src clean

install:
	install -Dm 755 src/gtkevemon $(DESTDIR)$(BINDIR)/gtkevemon
	$(MAKE) -C icon

uninstall:
	${RM} $(DESTDIR)$(BINDIR)/gtkevemon
	$(MAKE) -C icon uninstall
