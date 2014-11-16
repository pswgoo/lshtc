# include makefile basic and tools
include ../include_makefile/basic.mk
include ../include_makefile/tools.mk

# Project Name or Library Name 
PRO_EXE := 
PRO_LIB := $(BIN_DIR)$(notdir $(shell pwd)).a

# Libraries references
LIB_REFERENCES := $(BIN_DIR)extractfeature.a $(BIN_DIR)evaluation.a $(BIN_DIR)medline.a \
				$(BIN_DIR)libsvm.a $(BIN_DIR)liblinear.a $(BIN_DIR)jsoncpp.a $(BIN_DIR)common.a

include ../include_makefile/makefile.mk
