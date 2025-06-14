# Tool macros
CC := gcc
CCFLAGS := -Wall
DBGFLAGS := -Wall -Werror -g
CCOBJFLAGS := $(CCFLAGS) -c
LIBS := -lm -lncursesw -lpanelw

# Compile macros
TARGET_NAME := finterm

# Path macros
BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH := src
DBG_PATH := debug

# Find all .c files recursively in SRC_PATH
SRC := $(shell find $(SRC_PATH) -type f -name "*.c")

# Generate object file names with matching subdirectory structure under OBJ_PATH
OBJ := $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o,$(SRC))

# Default target
all: $(BIN_PATH)/$(TARGET_NAME)

# Link the target
$(BIN_PATH)/$(TARGET_NAME): $(OBJ)
	@mkdir -p $(BIN_PATH)
	$(CC) $(CCFLAGS) -o $@ $^ $(LIBS)

# Compile source files to object files
$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CCOBJFLAGS) -o $@ $<

# Debug build
.PHONY: debug
debug: CCFLAGS := $(DBGFLAGS)
debug: clean $(DBG_PATH)/$(TARGET_NAME)

$(DBG_PATH)/$(TARGET_NAME): $(OBJ)
	@mkdir -p $(DBG_PATH)
	$(CC) $(DBGFLAGS) -o $@ $^ $(LIBS)

# Clean
.PHONY: clean
clean:
	rm -rf $(OBJ_PATH) $(BIN_PATH) $(DBG_PATH)

# Phony target to just build objects
.PHONY: objects
objects: $(OBJ)

# Print variables (for debugging the makefile itself)
.PHONY: vars
vars:
	@echo "SRC files: $(SRC)"
	@echo "OBJ files: $(OBJ)"