include ../../config.mk

PROJECT_NAME = ginga

CFLAGS 	+= $(DFB_CFLAGS) -I../../build/include/lifia -I../telemidia-links -I.

# Add shared (dynamic) libraries, these are all our dependencies
LIBS += -lboost_system -lboost_thread -lboost_filesystem -lpng -ljpeg -ltiff -lgif -llua -lxerces-c -lssl -lcurl

LIBS += $(shell pkg-config --libs directfb)

# Add local (static) libraries to linker
LIBS_INTERNAL = ../dtv-connector/libdtvconnector.a ../gingacc-player/libdtvplayer.a ../dtv-util/libdtvutil.a ../gingacc-system/libdtvsystem.a ../gingacc-multidevice/libdtvmultidevice.a ../gingalssm/libdtvlssm.a ../telemidia-links/libtelemidia-links.a ../ncl30-presenter/libdtvncl-presenter.a ../gingacc-ic/libdtvic.a ../gingacc-contextmanager/libdtvcontextmanager.a ../ncl30-converter/libdtvncl-converter.a ../ncl30/libdtvncl.a

# define a suffix rule for .cpp -> .o
.cpp.o:
	$(Q)$(ECHO) "    CC    "$@
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

$(PROJECT_NAME): $(OBJECTS) $(LIBS_INTERNAL)
	$(ECHO) "    LD    "$@
	$(Q)$(LD) $(OBJECTS) -Wl,--start-group $(LIBS)  $(LIBS_INTERNAL) -Wl,--end-group -o $@
	$(Q)$(STRIP) -s $(PROJECT_NAME)

install: $(PROJECT_NAME)
	$(Q)mkdir -p ../../build/bin
	$(Q)cp $(PROJECT_NAME) ../../build/bin/
	$(Q)cp src/ginga.version ../../build
	
clean:
	$(Q)$(RM) -f $(PROJECT_NAME) $(OBJECTS)
