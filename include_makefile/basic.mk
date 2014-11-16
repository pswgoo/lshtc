
# Excute binary file or Library file Path, the follow dirs can be empty
BIN_DIR := ../bin_linux/
#OBJ_DIR := ./obj/

# Project Name or Library Name  $(BIN_DIR)makefile.a
PRO_EXE := 
PRO_LIB := 

# Filter out object
FILTER_OBJECT := 

# Libraries references
LIB_REFERENCES := 

# Include files
INCS := -I ../

# Include LIB_REFERENCES Makefile base dir
LIB_MK_BASE_DIR := ../

# C++ Compiler
CXX := g++
# C++ Compiler options during compilation  -lstdc++
CXX_FLAGS := -O2 -Wall -std=gnu++0x -m64 -fopenmp

# C Compiler
CC := gcc
# C Compiler options during compilation
CC_FLAGS := -O2 -Wall -m64 -fopenmp

# Static library use 'ar' command 
AR := ar
RANLIB := ranlib

# AR options
AR_FLAGS := -rv

# Libraries for linking
LIBS := -lstdc++ -lm -fopenmp
