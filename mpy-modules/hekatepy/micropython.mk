CEXAMPLE_MOD_DIR := $(USERMOD_DIR)

# target_include_directories(basicstation PRIVATE /mnt/d/Development/Repos/hekate/external/micropython/lib/mbedtls/include)
CFLAGS_USERMOD += -I$(CEXAMPLE_MOD_DIR)/../../hekatelib/include
# CFLAGS_USERMOD += -I$(CEXAMPLE_MOD_DIR)/lib/mbedtls/include
LDFLAGS_USERMOD += -L$(CEXAMPLE_MOD_DIR)/../../build/hekatelib -lhekatelib_static


# Add all C files to SRC_USERMOD.
SRC_USERMOD += $(CEXAMPLE_MOD_DIR)/hekatepy.c

# We can add our module folder to include paths if needed
# This is not actually needed in this example.
CFLAGS_USERMOD += -I$(CEXAMPLE_MOD_DIR)
