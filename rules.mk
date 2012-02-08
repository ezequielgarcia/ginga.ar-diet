
###############################################################################
## YOU CAN'T TOUCH THIS (UNLESS YOU ARE ME)

all: $(STATIC_OUT)

# define a rule for static library
$(STATIC_OUT): $(OBJECTS)
	$(Q)$(AR) -r $@ $^
	$(Q)$(AR) -s $@

# define a suffix rule for .cpp -> .o
.cpp.o:
	$(Q)$(ECHO) "    CC    "$@;
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

# define a suffix rule for .c -> .o
.c.o:
	$(Q)$(ECHO) "    CC    "$@;
	$(Q)$(CC) $(CFLAGS) -c $<

install_headers: ../../build/include/lifia/$(PROJECT_NAME)
../../build/include/lifia/$(PROJECT_NAME):
	$(Q)mkdir -p ../../build/include/lifia/$(PROJECT_NAME)
	$(Q)cd include/ && cp -a --parents $(HEADERS:include/%=%) ../../../build/include/lifia/$(PROJECT_NAME) 

install: install_headers
	$(Q)mkdir -p ../../build/lib/lifia
	$(Q)cp -a $(STATIC_OUT) ../../build/lib/lifia

clean:
	$(Q)$(RM) -f $(STATIC_OUT) $(OBJECTS)

clean_headers:
	$(Q)find ../../build/include/lifia/ -name "*.h" | xargs rm -rf
