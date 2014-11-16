# include makefile basic and tools

MKFILE_DIR := $(dir $(shell find ./*/Makefile))

all: 
	@echo Reconstructing
	@echo $(MKFILE_DIR)
	@: $(foreach dir, $(MKFILE_DIR), $(shell cd $(dir) && make))

.PHONY: clean
clean:
	@echo Clean
	@: $(foreach dir, $(MKFILE_DIR), $(shell cd $(dir) && make clean))


