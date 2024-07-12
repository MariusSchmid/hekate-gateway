CEXAMPLE_MOD_DIR := $(USERMOD_DIR)


CFLAGS_USERMOD += -I$(CEXAMPLE_MOD_DIR)/../../include
# LDFLAGS_USERMOD += -L$(CEXAMPLE_MOD_DIR)/../../build_mpy_unix/hekatelib/external -lbasicstation
LDFLAGS_USERMOD += -L$(CEXAMPLE_MOD_DIR)/../../build_mpy_unix -lconcentrator


# Add all C files to SRC_USERMOD.
SRC_USERMOD += $(CEXAMPLE_MOD_DIR)/concentratorpy.c

# We can add our module folder to include paths if needed
# This is not actually needed in this example.
CFLAGS_USERMOD += -I$(CEXAMPLE_MOD_DIR)
