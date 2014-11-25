
# Subdirs to search for additional source files
SUBDIRS ?= $(shell ls -F | grep "\/" )
DIRS ?= ./ $(SUBDIRS)
CXX_SOURCE_FILES ?= $(foreach d, $(DIRS), $(wildcard $(d)*.cpp))
CC_SOURCE_FILES ?= $(foreach d, $(DIRS), $(wildcard $(d)*.c))

# Create an object file of every .cpp file
CPP_OBJECTS ?= $(filter-out $(FILTER_OBJECT), $(patsubst %.cpp, %.cpp.o, $(CXX_SOURCE_FILES)))
# Create an object file of every .c file
CC_OBJECTS ?= $(filter-out $(FILTER_OBJECT), $(patsubst %.c, %.c.o, $(CC_SOURCE_FILES)))

# Create dependent file for each object file
DEPEND_FILES ?= $(CPP_OBJECTS:.o=.d)
DEPEND_FILES += $(CC_OBJECTS:.o=.d)

# Update LIB_REFERENCES recursively
DIRECT_REFER := $(LIB_REFERENCES)
LIB_REFERENCES += $(call CALL_MAKEFILES,$(call GET_BASENAMES,$(DIRECT_REFER)),$(LIB_MK_BASE_DIR),print_reference)
LIB_REFERENCES := $(sort $(LIB_REFERENCES))

.PHONY: all
# Make $(PRO_EXE) $(PRO_LIB) the default target
all: $(PRO_EXE) $(PRO_LIB) $(LIB_REFERENCES)

# Use -Xlinker to ignore  the library refer order
$(PRO_EXE): $(CPP_OBJECTS) $(CC_OBJECTS) $(LIB_REFERENCES)
	@mkdir -p $(BIN_DIR)
	$(CXX) -o $@  $(CPP_OBJECTS) $(CC_OBJECTS) -Xlinker "-(" $(LIB_REFERENCES) -Xlinker "-)" $(LIBS)

$(PRO_LIB): $(CPP_OBJECTS) $(CC_OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(AR) $(AR_FLAGS) $@ $^
	$(RANLIB) $(PRO_LIB)

# Compile every cpp file to an object
%.cpp.o: %.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCS) -o $@ $<
	
# Generate header dependence file: *.d for each *.c
%.cpp.d: %.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $< $(INCS) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# Compile every c file to an object
%.c.o: %.c
	$(CC) -c $(CC_FLAGS) $(INCS) -o $@ $<

# Generate header dependence file: *.d for each *.c
%.c.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $< $(INCS) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
	
.PHONY: tag_update
# Update Library References
$(LIB_REFERENCES): tag_update
	@set -e; : $(call CALL_MAKEFILES,$(call GET_BASENAMES,$@),$(LIB_MK_BASE_DIR))

# Include header dependence file: *.d for each *.cpp
-include $(DEPEND_FILES)

# Print library references
print_reference: 
	@echo $(LIB_REFERENCES)
	
# Clean
clean:
	rm -f $(PRO_EXE) $(PRO_LIB) $(CPP_OBJECTS) $(CC_OBJECTS) $(DEPEND_FILES) $(patsubst %.d, %.d.*, $(DEPEND_FILES))

.PHONY: clean
