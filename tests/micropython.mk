EXAMPLE_MOD_DIR := $(USERMOD_DIR)
SRC_USERMOD += $(EXAMPLE_MOD_DIR)/module.c
LDFLAGS_USERMOD += $(EXAMPLE_MOD_DIR)/libupywraptest.a -lstdc++