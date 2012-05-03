#
# MurkMUD++ - A Windows compatible, C++ compatible Merc 2.2 Mud.
#
# author Jon A. Lambert
# date 01/02/2007
# version 1.5
# remarks
#  This source code copyright (C) 2005, 2006, 2007 by Jon A. Lambert
#  All rights reserved.
#
#  Use governed by the MurkMUD++ public license found in license.murk++

#
# Murk++ build for cygwin, linux and bsd
#
CPP = g++
CC = gcc
AR = ar

# The suffix appended to executables.  
# This should be set for Cygwin.
#EXE = .exe
#EXE =

OPTIM = -O2 -pipe 
WARN = -Wall -Wno-parentheses -Wno-unused 
PROF    = -g

CPPFLAGS = $(OPTIM) $(WARN) -W $(PROF) -fno-default-inline 
CFLAGS = $(OPTIM) $(DEFS) $(WARN) -fno-strict-aliasing
LFLAGS = $(OPTIM) $(PROF) 

INCS = -Isqlite3 
LIBS = -lcrypt -ldl -lpthread

SQLITE_SRC = sqlite3/sqlite3.c
SQLITE_OBJ = $(SQLITE_SRC:.c=.o)
SQLITE_LIB = sqlite3/libsqlite3.a

SQLITE_PRG_SRC = sqlite3/shell.c 
SQLITE_PRG_OBJ = $(SQLITE_PRG_SRC:.c=.o)
SQLITE_PRG = sqlite3/sqlite3$(EXE) 

SQLITE_XTRA = sqlite3/sqlite3.h sqlite3/sqlite3ext.h 

MURK_UTIL_SRC = loadhelps.cpp
MURK_UTIL_OBJ = $(MURK_UTIL_SRC:.cpp=.o)
MURK_SRC = murk.cpp os.cpp descriptor.cpp utils.cpp commands.cpp io.cpp \
	room.cpp object.cpp character.cpp spells.cpp objproto.cpp mobproto.cpp \
	affect.cpp exit.cpp area.cpp reset.cpp extra.cpp shop.cpp pcdata.cpp \
	symbols.cpp database.cpp world.cpp
MURK_OBJ = $(MURK_SRC:.cpp=.o)
MURK_HDR = os.hpp config.hpp descriptor.hpp character.hpp pcdata.hpp utils.hpp \
	globals.hpp object.hpp note.hpp room.hpp area.hpp mobproto.hpp \
	objproto.hpp affect.hpp exit.hpp extra.hpp ban.hpp shop.hpp reset.hpp \
	io.hpp symbols.hpp database.hpp spell_list.hpp cmd_list.hpp \
	baseobject.hpp world.hpp

OBJDEPENDS = $(MURK_OBJ) $(MURK_UTIL_OBJ)

# Data files Areas, Mobprogs and Players
DATAFILES = area.lst limbo.are mid_cit.prg midgaard.are school.are help.are \
        vagabond.prg beggar.prg crier.prg drunk.prg gategrd.prg gategrd2.prg \
	janitor.prg One schema item_types socials titles

# Files in the standard distribution
DISTFILES = $(MURK_SRC) $(MURK_UTIL_SRC) $(MURK_HDR) $(DATAFILES) \
	$(SQLITE_SRC) $(SQLITE_PRG_SRC) $(SQLITE_XTRA) \
	makefile makefile.bor makefile.vc makefile.dgm \
	doc.txt startup startup.bash startup.cmd \
	license.crypt license.diku license.merc license.murk++ 
  
PDIST= $(patsubst %,murk++/%,$(DISTFILES))
RELEASE=dist

TARGETS = $(SQLITE_LIB) $(SQLITE_PRG) murk$(EXE) loadhelps$(EXE)

all: $(TARGETS)

$(SQLITE_LIB): $(SQLITE_OBJ)
	$(AR) rsc $@ $^

$(SQLITE_PRG): $(SQLITE_PRG_OBJ) $(SQLITE_LIB)
	$(CC) $(LFLAGS) -o $@ $^ $(LIBS)

murk$(EXE): $(MURK_OBJ) $(SQLITE_LIB) 
	@-rm murkold$(EXE)
	@-mv murk$(EXE) murkold$(EXE)
	$(CPP) $(LFLAGS) -o $@ $^ $(LIBS)

loadhelps$(EXE): loadhelps.o io.o $(SQLITE_LIB) 
	$(CPP) $(LFLAGS) -o $@ $^ $(LIBS)

database: $(SQLITE_PRG) loadhelps$(EXE)
	@echo "Building database..."
	@-cp murk.db murk.db.bkup
	@-rm murk.db
	@sqlite3/sqlite3 murk.db < schema
	@./loadhelps
	@echo "Done."

clean:
	-rm -f murk$(EXE) loadhelps$(EXE) $(MURK_OBJ) $(MURK_UTIL_OBJ) \
	$(OBJDEPENDS:.o=.d)

cleanall:
	-rm -f $(TARGETS) $(MURK_OBJ) $(MURK_UTIL_OBJ) $(SQLITE_OBJ) \
	$(SQLITE_PRG_OBJ) $(OBJDEPENDS:.o=.d)

dist:
	ln -s ./ murk++
	tar czvf murk++-$(RELEASE).tar.gz $(PDIST)
	rm murk++
	
# pull in dependency info for *existing* .o files
-include $(OBJDEPENDS:.o=.d)

# compile and generate dependency info;
# more complicated dependency computation, so all prereqs listed
# will also become command-less, prereq-less targets
#   sed:    append directory to object target. (gcc bug?)
#   sed:    strip the target (everything before colon)
#   sed:    remove any continuation backslashes
#   fmt -1: list words one per line
#   sed:    strip leading spaces
#   sed:    add trailing colons
%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $(INCS) $*.cpp -o $*.o
	@$(CPP) -MM $(CPPFLAGS) $(INCS) $*.cpp > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

#.cpp.o: 
#	$(CPP) -c $(CPPFLAGS) $(INCS) $*.cpp -o $*.o 

.c.o: 
	$(CC) -c $(CFLAGS) $(INCS) $*.c -o $*.o 

