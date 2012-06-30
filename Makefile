#
# Makefile
# Copyright (C) 2009-2011, 2012 Joachim de Groot <jdegroot@web.de>
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be usefuse along
# with this program.  If not, see <http://www.gnu.org/licenses/>.
#

.PHONY: help clean dist

ifndef config
  config=debug
endif

OS   := $(shell uname -s)
ARCH := $(shell uname -m)

# check if operating on a git checkout
ifneq ($(wildcard .git),)
  # get hash of current commit
  GITREV := $(shell git show --pretty=oneline | sed -ne '1 s/\(.\{6\}\).*/\1/p')
  # get tag for current commit
  GITTAG := $(shell git tag --contains $(GITREV) | tr '[:upper:]' '[:lower:]')
  ifeq ($(GITTAG),)
    # current commit is not tagged
    # -> include current commit hash in version information
    DEFINES += -DGITREV=\"-g$(GITREV)\"
  endif
endif

VERSION_MAJOR := $(shell awk '/VERSION_MAJOR/ { print $$3 }' inc/nlarn.h)
VERSION_MINOR := $(shell awk '/VERSION_MINOR/ { print $$3 }' inc/nlarn.h)
VERSION_PATCH := $(shell awk '/VERSION_PATCH/ { print $$3 }' inc/nlarn.h)

# Collect version information
ifneq ($(GITTAG),)
  # Remove "nlarn-" from the tag name as it will be prepended later.
  VERSION := $(patsubst nlarn-%,%,$(GITTAG))
else
  # not on a release tag, determine version manually
  VERSION = $(VERSION_MAJOR).$(VERSION_MINOR)

  ifneq ($(VERSION_PATCH),0)
	VERSION := $(VERSION).$(VERSION_PATCH)
  endif

  ifneq ($(GITREV),)
    date     = $(shell date "+%Y%m%d")
    VERSION := $(VERSION)-$(date)-g$(GITREV)
  endif
endif

# Definitions required regardless of host OS
DEFINES += -DG_DISABLE_DEPRECATED
CFLAGS  += -MMD -MP -std=c99 -Wall -Wextra -Werror -Iinc
LDFLAGS += -lz

ifeq ($(MSYSTEM),MINGW32)
  # Settings specific to Windows.

  # Fake the content of the OS var to make it more common
  # (otherwise packages would have silly names)
  OS := win32

  # Ensure make doesn't try to run cc
  CC = gcc

  RESOURCES := $(patsubst %.rc,%.res,$(wildcard resources/*.rc))
  RESDEFINE := -DVERSION_MAJOR=$(VERSION_MAJOR)
  RESDEFINE += -DVERSION_MINOR=$(VERSION_MINOR)
  RESDEFINE += -DVERSION_PATCH=$(VERSION_PATCH)
  # Escape-O-Rama! Required in all it's ugliness.
  RESDEFINE += -DVINFO=\\\"$(VERSION)\\\"

  # Libraries specific to Windows
  LDFLAGS += -static -lpdcurses -llua

  # Configuration for glib-2
  # Funny enough, build breaks if these are set as ususal..
  CFLAGS  += `pkg-config --cflags glib-2.0`
  LDFLAGS += `pkg-config --libs glib-2.0`

  # Defines specific to Windows
  DEFINES += -DWIN32_LEAN_AND_MEAN -DNOGDI

  # and finally the dreaded executable suffix from the eighties
  SUFFIX = .exe
  ARCHIVE_CMD = zip -r
  ARCHIVE_SUFFIX = zip
  INSTALLER := nlarn-$(VERSION).exe

else
  # Settings specific to Un*x-like operating systems

  # Configuration for glib-2
  CFLAGS  += $(shell pkg-config --cflags glib-2.0)
  LDFLAGS += $(shell pkg-config --libs glib-2.0)

  # Configuration for ncurses
  ifeq ($(filter Darwin DragonFly OpenBSD,$(OS)),)
    CFLAGS  += $(shell ncurses5-config --cflags)
    LDFLAGS += $(shell ncurses5-config --libs) -lpanel
  else
    # OS X is handled separately
    ifneq ($(OS), Darwin)
      # DragonFly and OpenBSD have ncurses in base (and no config tool)
      LDFLAGS += -lncurses -lpanel
    endif
  endif

  # Determine the name of the Lua 5.1 library
  # Debian and derivates use lua5.1, the rest of the world lua
  ifneq ($(wildcard /etc/debian_version),)
    lua = lua5.1
  else
    lua = lua
  endif

  # Configure Lua
  CFLAGS  += $(shell pkg-config --cflags $(lua))
  LDFLAGS += $(shell pkg-config --libs $(lua))

  # executables on other plattforms do not have a funny suffix
  SUFFIX =
  ARCHIVE_CMD = tar czf
  ARCHIVE_SUFFIX = tar.gz
endif

# Enable creating packages when working on a git checkout
ifneq ($(GITREV),)
  DIRNAME   = nlarn-$(VERSION)
  SRCPKG    = nlarn-$(VERSION).tar.gz
  PACKAGE   = $(DIRNAME)_$(OS).$(ARCH).$(ARCHIVE_SUFFIX)
  MAINFILES = nlarn$(SUFFIX) nlarn.ini-sample README.txt LICENSE
  LIBFILES  = lib/fortune lib/maze lib/maze_doc.txt lib/nlarn.* lib/*.lua
endif

ifeq ($(OS),Darwin)
  # Use clang on OS X.
  CC = clang
  # Unless requested otherwise build with SDL PDCurses on OS X.
  ifeq ($(NCURSES),)
    CFLAGS  += -DSDLPDCURSES $(shell sdl-config --cflags) -Dmain=SDL_main
    LDFLAGS += -lpdcurses $(shell sdl-config --static-libs)
  endif
  ifneq ($(NCURSES),)
    LDFLAGS += -lcurses -lpanel
  endif
  OSXIMAGE := nlarn-$(VERSION).dmg
endif

ifeq ($(config),debug)
  DEFINES   += -DDEBUG -DLUA_USE_APICHECK
  CFLAGS    += $(DEFINES) -g
  RESFLAGS  += $(DEFINES) $(INCLUDES)
endif

ifeq ($(config),release)
  DEFINES   += -DG_DISABLE_ASSERT
  CFLAGS    += $(DEFINES) -O2
  RESFLAGS  += $(DEFINES) $(INCLUDES)
endif

OBJECTS := $(patsubst %.c,%.o,$(wildcard src/*.c))

all: nlarn$(SUFFIX)

nlarn$(SUFFIX): $(OBJECTS) $(RESOURCES)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(RESOURCES)

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(RESOURCES): %.res: %.rc
	windres -v $(RESDEFINE) $< -O coff -o $@

dist: clean $(SRCPKG) $(PACKAGE) $(INSTALLER) $(OSXIMAGE)

$(SRCPKG):
	@echo -n Packing source archive $(SRCPKG)
	@git archive --prefix $(DIRNAME)/ --format=tar $(GITREV) | gzip > $(SRCPKG)
	@echo " - done."

$(PACKAGE): nlarn$(SUFFIX)
	@echo -n Packing $(PACKAGE)
	@mkdir -p $(DIRNAME)/lib
	@cp -p $(MAINFILES) $(DIRNAME)
	@cp -p $(LIBFILES) $(DIRNAME)/lib
	@$(ARCHIVE_CMD) $(PACKAGE) $(DIRNAME)
	@rm -rf $(DIRNAME)
	@echo " - done."

# The Windows installer
$(INSTALLER): nlarn$(SUFFIX) nlarn.nsi
	@echo -n Packing $(PACKAGE)
	@makensis //DVERSION="$(VERSION)" \
		//DVERSION_MAJOR=$(VERSION_MAJOR) \
		//DVERSION_MINOR=$(VERSION_MINOR) \
		//DVERSION_PATCH=$(VERSION_PATCH) nlarn.nsi
	@echo " - done."

# The OSX installer
$(OSXIMAGE): nlarn
	@mkdir -p dmgroot/NLarn.app/Contents/{Frameworks,MacOS,Resources}
	@cp -p nlarn dmgroot/NLarn.app/Contents/MacOS
# Copy local libraries into the app folder and instruct the linker.
	@for lib in $$(otool -L nlarn | awk '/local/ {print $$1}'); do\
		cp $$lib dmgroot/NLarn.app/Contents/MacOS/; \
		chmod 0644 dmgroot/NLarn.app/Contents/MacOS/$${lib##*/}; \
		install_name_tool -change $$lib @executable_path/$${lib##*/} \
			dmgroot/NLarn.app/Contents/MacOS/nlarn; \
	done
# Copy required files
	@cp -p lib/{fortune,maze,monsters.lua,nlarn*} \
		dmgroot/Nlarn.app/Contents/Resources
	@cp -p resources/NLarn.icns dmgroot/NLarn.app/Contents/Resources
	@cp -pr Changelog.txt README.txt LICENSE dmgroot
	@cp -p resources/Info.plist dmgroot/NLarn.app/Contents
# Update the version information in the plist
	/usr/libexec/PlistBuddy -c "Set :CFBundleVersion $(VERSION)" \
		dmgroot/NLarn.app/Contents/Info.plist
	/usr/libexec/PlistBuddy -c \
		"Add :CFBundleShortVersionString string $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)" \
		dmgroot/NLarn.app/Contents/Info.plist
# Use the same icons for the dmg file
	@cp -p resources/NLarn.icns dmgroot/.VolumeIcon.icns
	@SetFile -c icnC dmgroot/.VolumeIcon.icns
# Create a pseudo-installer
	@ln -s "/Applications" dmgroot/Applications
	@cp resources/dmg_background.png dmgroot/.background.png
	@cp resources/dot.DS_Store dmgroot/.DS_store
# Create the disk image
	@echo hditool requires superuser rights.
	@sudo hdiutil create -srcfolder dmgroot -volname "NLarn $(VERSION)" \
		-uid 99 -gid 99 -format UDRW "raw-$(DIRNAME).dmg"
	@rm -rf dmgroot
	@mkdir dmgroot
	@sudo hdiutil attach -readwrite "raw-$(DIRNAME).dmg" -mountpoint dmgroot
	@sudo SetFile -a C dmgroot
	@sudo hdiutil detach dmgroot
	@rm -rf dmgroot
	@sudo hdiutil convert "raw-$(DIRNAME).dmg" -format UDZO -o "$(DIRNAME).dmg"
	@sudo rm "raw-$(DIRNAME).dmg"

clean:
	@echo Cleaning nlarn
	rm -f nlarn
	rm -f $(RESOURCES) $(OBJECTS) $(patsubst %.o,%.d,$(OBJECTS))
	rm -f $(SRCPKG) $(PACKAGE) $(INSTALLER) $(OSXIMAGE)

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "   debug"
	@echo "   release"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default) - builds nlarn$(SUFFIX)"
	@echo "   clean         - cleans the working directory"
	@if \[ -n "$(GITREV)" \]; then \
		echo "   dist          - create source and binary packages for distribution"; \
		echo "                   ($(SRCPKG) and $(PACKAGE))"; \
	fi
	@echo ""

-include $(OBJECTS:%.o=%.d)
