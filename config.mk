###############################################################################
## YOU CAN TOUCH THESE (OR YOU CAN OVERRIDE IT IN SUBDIR MAKEFILE)

PREFIX_PATH = "\"/home/zeta/ginga\""

CC  = g++
AR  = ar
LD  = $(CC)
RM  = rm
STRIP  = strip
Q = @
ECHO = @echo

CFLAGS = -Wall -O2 -DPREFIX_PATH=$(PREFIX_PATH)
DFB_CFLAGS = $(shell pkg-config --cflags directfb)
LUA_CFLAGS = $(shell pkg-config --cflags lua)


###############################################################################
## YOU CAN'T TOUCH THIS (UNLESS YOU ARE ME)

SOURCES_DIRS = $(shell find src -type d -print)

STATIC_OUT = libdtv$(PROJECT_NAME).a

HEADERS = $(shell find -H include -name "*.h")
SOURCES = $(foreach dir, $(SOURCES_DIRS), $(wildcard $(dir)/*.cpp))
OBJECTS = $(SOURCES:.cpp=.o)
