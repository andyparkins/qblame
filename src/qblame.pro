# ----------------------------------------------------------------------------
# Project: qblame
#  @file   qblame.pro
#  @author Andy Parkins
#
# Version Control
#    $Author$
#      $Date$
#        $Id$
#
# Legal
#    Copyright 2006  Andy Parkins
#
# ----------------------------------------------------------------------------

# --- Config
TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

# --- Input
HEADERS += qblame.h
FORMS += qblame.ui
SOURCES += qblame.cc

# --- Dependencies
PRE_TARGETDEPS += tags


# -------------- Extras
# --- Custom make recipes
QMAKE_EXTRA_TARGETS = build run gdb grind callgrind tags

# Build
build.depends = ./$(TARGET)

# Run
unix {
	run.commands = rm -f core*; ./$(TARGET) qblame.pro > debug.out
}
run.depends = ./$(TARGET)

# Debug
gdb.commands = gdb ./$(TARGET)
gdb.depends = ./$(TARGET)

# Valgrind
grind.commands = rm -f vg-$(TARGET)*; valgrind --leak-check=yes --log-file=vg-$(TARGET) ./$(TARGET) > debug.out
grind.depends = ./$(TARGET)

# Callgrind
callgrind.commands = rm -f callgrind.out.*; callgrind --separate-threads=yes --separate-callers=2 ./$(TARGET) > debug.out
callgrind.depends = ./$(TARGET)

# Exuberant ctags
tags.depends = $(HEADERS) $(SOURCES)
tags.commands = ctags --language-force=c++ --extra=+q --fields=+i $(HEADERS) $(SOURCES)

# --------------- External objects
#libs.commands = make -C ../lib

