SHELL = /bin/sh

# Determine platform
ifeq (${OS},Windows_NT)
  PLATFORM = win32
else
  PLATFORM = $(shell uname|tr '[:upper:]' '[:lower:]')
endif
#include Makefile.$(PLATFORM)

# Compiler options, flags, and libraries
GCC_FLAGS = -Wall -Wextra -ansi -Wundef -Wconversion -pedantic -Os
ifdef DEBUG
    GCC_FLAGS := $(filter-out -Os,$(GCC_FLAGS))
    GCC_FLAGS += -g -O0
endif

GCC_INCL = -I.
GTK_FLAGS = $(shell pkg-config --cflags gtkmm-2.4)
XML_FLAGS = $(shell pkg-config --cflags libxml-2.0)
SSL_FLAGS = $(shell pkg-config --cflags openssl)

GTK_LIBS = $(shell pkg-config --libs gtkmm-2.4)
XML_LIBS = $(shell pkg-config --libs libxml-2.0)
SSL_LIBS = $(shell pkg-config --libs openssl)
PTH_LIBS = -lpthread
ZLIB_LIBS = -lz
CURL_LIBS = $(shell pkg-config --libs libcurl)

# Platform specific configuration
ifeq (${PLATFORM},darwin)
  CXX ?= g++
  LDFLAGS = ${GTK_LIBS} ${PTH_LIBS} ${XML_LIBS} ${ZLIB_LIBS} ${SSL_LIBS}

  CORES_LINE = $(shell system_profiler SPHardwareDataType | grep Cores)
  CORES = $(lastword $(CORES_LINE))
  SOURCES = util/pipedexec_posix.cc util/os_unix.cc
  BINARY = gtkevemon
endif

ifeq (${PLATFORM},freebsd)
  CXX ?= g++
  PTH_LIBS = -pthread
  LDFLAGS = ${GTK_LIBS} ${PTH_LIBS} ${XML_LIBS} ${ZLIB_LIBS} ${SSL_LIBS}

  CORES = $(shell sysctl -n hw.ncpu)
  SOURCES = util/pipedexec_posix.cc util/os_unix.cc
  BINARY = gtkevemon
endif

ifeq (${PLATFORM},linux)
  CXX ?= g++
  LDFLAGS = ${GTK_LIBS} ${PTH_LIBS} ${XML_LIBS} ${ZLIB_LIBS} ${SSL_LIBS} ${CURL_LIBS}

  CORES = $(shell grep processor /proc/cpuinfo | wc -l)
  SOURCES = util/pipedexec_posix.cc util/os_unix.cc
  BINARY = gtkevemon
endif

ifeq (${PLATFORM},sunos)
  CXX ?= sunCC
  GCC_FLAGS = +w +w2 -g
  SOLARIS_LIBS = -lnsl -lresolv -lsocket
  LDFLAGS = ${GTK_LIBS} ${PTH_LIBS} ${SOLARIS_LIBS} ${XML_LIBS} ${ZLIB_LIBS} ${SSL_LIBS}

  CORES = $(shell psrinfo -p)
  SOURCES = util/pipedexec_posix.cc util/os_unix.cc util/timegm.cc
  BINARY = gtkevemon
endif

ifeq (${PLATFORM},win32)
  CXX ?= i586-mingw32msvc-g++
  LDFLAGS = ${GTK_LIBS} ${PTH_LIBS} ${XML_LIBS} ${SSL_LIBS}
  CORES = 1

  SOURCES = util/pipedexec_win32.cc util/os_win32.cc util/strptime.cc util/timegm.cc
  BINARY = gtkevemon.exe
endif

# Generic compiler flags
CXXFLAGS ?= ${GCC_FLAGS}
CXXFLAGS += ${GTK_FLAGS} ${XML_FLAGS} ${SSL_FLAGS} ${GCC_INCL}

# Source and object files
SOURCES += util/bgprocess.cc util/conf.cc util/helpers.cc \
           $(wildcard api/[^_]*.cc) $(wildcard net/[^_]*.cc) \
		   $(wildcard gui/[^_]*.cc) $(wildcard bits/[^_]*.cc) \
		   gtkevemon.cc
OBJECTS = $(foreach file,$(SOURCES),$(subst .cc,.o,$(file)))
DEPENDENCIES = $(foreach file,$(SOURCES),$(subst .cc,.DEP,$(file)))

#### Building targets ####

all:
	$(MAKE) -j${CORES} gtkevemon

debug:
	$(MAKE) -j${CORES} DEBUG=1 gtkevemon

gtkevemon: ${OBJECTS}
	${CXX} -o ${BINARY} ${OBJECTS} ${LDFLAGS}

gemcache:
	${RM} gemcache
	${CXX} -o gemcache gemcache.cc ${CXXFLAGS}

%.o: %.cc
	${CXX} -c -o $@ $< ${CXXFLAGS}

#test: ${OBJECTS}
#	${CXX} -o _test_ssl _test_ssl.cc net/*.o util/helpers.o ${LDFLAGS} ${GCC_INCL}

#### Dependencies target ####

depend: depend-clean ${DEPENDENCIES}

depend-clean:
	${RM} Makefile.dep

%.DEP:
	${CXX} -MM -MT $(subst .DEP,.o,$@) $(subst .DEP,.cc,$@) ${GCC_INCL} >> Makefile.dep

#### Cleaning target ####

clean: FORCE
	${RM} ${BINARY} ${OBJECTS}
	${RM} gemcache

FORCE:

#### Compile time dependency information ####

-include Makefile.dep
