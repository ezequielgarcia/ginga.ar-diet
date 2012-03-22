
LIBDIRS = dtv-connector dtv-util gingacc-system gingacc-ic gingacc-player gingalssm gingacc-contextmanager gingacc-multidevice ncl30 ncl30-presenter ncl30-converter telemidia-links

BUILD_LIBDIRS = $(LIBDIRS:%=build-%)
CLEAN_DIRS = clean-ginga $(LIBDIRS:%=clean-%)

MAKE_BUILD = $(MAKE) -s -C src/$(@:build-%=%)
MAKE_CLEAN = $(MAKE) -s -C src/$(@:clean-%=%)

all: build-ginga $(BUILD_LIBDIRS)
$(BUILD_LIBDIRS):
	@echo "  Building library "$(@:build-%=%)
	@$(MAKE_BUILD)
	@echo "  Installing library "$(@:build-%=%)
	@$(MAKE_BUILD) install

clean: $(CLEAN_DIRS)
$(CLEAN_DIRS): 
	@echo "  Cleaning "$(@:clean-%=%)
	@$(MAKE_CLEAN) clean

build-ginga: $(BUILD_LIBDIRS)
	@echo "  Building ginga"
	@$(MAKE_BUILD)
	@echo "  Installing ginga"
	@$(MAKE_BUILD) install

distclean:
	@rm build -rf

# Dependencies
build-dtv-connector: build-dtv-util
build-gingacc-contextmanager: build-dtv-util build-gingacc-system
build-gingacc-ic: build-dtv-util
build-gingacc-multidevice: build-dtv-util build-gingacc-system
build-gingacc-player: build-dtv-util build-gingacc-system build-dtv-connector 
build-gingacc-system: build-dtv-util build-dtv-connector
build-gingalssm: build-dtv-util build-dtv-connector build-ncl30 build-ncl30-converter build-ncl30-presenter build-gingacc-player build-gingacc-system
build-ncl30: build-dtv-util build-gingacc-system
build-ncl30-converter: build-dtv-util build-ncl30
build-ncl30-presenter: build-dtv-util build-dtv-connector build-ncl30 build-ncl30-converter build-gingacc-system build-gingacc-ic build-gingacc-player build-gingacc-contextmanager build-gingacc-multidevice

.PHONY: all clean subdirs ginga $(BUILD_LIBDIRS) $(CLEAN_DIRS)
