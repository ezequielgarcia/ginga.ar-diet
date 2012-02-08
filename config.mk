###############################################################################
## YOU CAN TOUCH THESE (OR YOU CAN OVERRIDE IT IN SUBDIR MAKEFILE)

CC  = sh4-linux-g++
AR  = sh4-linux-ar
LD  = $(CC)
RM  = rm
STRIP  = sh4-linux-strip
Q = @
ECHO = @echo

CFLAGS = -Wall -O2
DFB_CFLAGS = $(shell pkg-config --cflags directfb)
LUA_CFLAGS = $(shell pkg-config --cflags lua)

###############################################################################
## YOU CAN'T TOUCH THIS (UNLESS YOU ARE ME)

SOURCES_DIRS = $(shell find src -type d -print)

STATIC_OUT = libdtv$(PROJECT_NAME).a

HEADERS = $(shell find -H include -name "*.h")
SOURCES = $(foreach dir, $(SOURCES_DIRS), $(wildcard $(dir)/*.cpp))
OBJECTS = $(SOURCES:.cpp=.o)
