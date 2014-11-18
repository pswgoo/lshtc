
# Update Library References, we only get the filename and ignore the suffix
# $(1) is a string of dirs
GET_BASENAMES = $(basename $(notdir $(1)))

# Call Makefile, call base_dir/project_names/Makefile 
# $(1) is project_names 
# $(2) is base_dir
# $(3) make parameters 
CALL_MAKEFILES = $(foreach d, $(foreach m, $(1), $(addprefix $(2),$(m))), $(shell cd $(d) && make) $(3))
