# include makefile basic and tools
include ../include_makefile/basic.mk
include ../include_makefile/tools.mk

# Project Name or Library Name 
PRO_EXE := 
PRO_LIB := $(BIN_DIR)$(notdir $(shell pwd)).a

FILTER_OBJECT := ./train.c.o ./predict.c.o ./svm-scale.c.o\
				train.c.o predict.c.o svm-scale.c.o

# Libraries references CC_FLAGS += -std=gnu++0x
LIB_REFERENCES := 

include ../include_makefile/makefile.mk
