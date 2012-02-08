include ../../config.mk

PROJECT_NAME = telemidia-links
STATIC_OUT = lib$(PROJECT_NAME).a

# Here we will override a bunch of variables from config.mk
CC  = sh4-linux-gcc
LD  = $(CC)

CFLAGS 	= -O2 $(DFB_CFLAGS) -DHAVE_CONFIG_H -I.

SOURCES = $(wildcard *.c)
OBJECTS = ${SOURCES:.c=.o}

all: $(STATIC_OUT)

# define a rule for static library
$(STATIC_OUT): $(OBJECTS)
	$(Q)$(AR) -r $@ $^
	$(Q)$(AR) -s $@

# define a suffix rule for .c -> .o
.c.o:
	$(Q)$(ECHO) "    CC    "$@;
	$(Q)$(CC) $(CFLAGS) -c $<

install:
	$(Q)mkdir -p ../../build/lib/lifia
	$(Q)cp -a $(STATIC_OUT) ../../build/lib/lifia

clean:
	$(Q)$(RM) -f $(STATIC_OUT) $(OBJECTS)
