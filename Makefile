# tool macros
CC := gcc# FILL: the compiler
CCFLAGS := -Wall # FILL: compile flags
DBGFLAGS := -Wall -Werror -g
CCOBJFLAGS := $(CCFLAGS) -c
LIBS := -lm -lncursesw -lpanel

# compile macros
TARGET_NAME := finterm# FILL: target name

# path macros
BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH := src
DBG_PATH := debug
LIB_PATH := $(BIN_PATH)/lib
LIB_HEADER_PATH := $(LIB_PATH)/$(TARGET_NAME)

ifeq ($(OS),Windows_NT)
	TARGET_NAME := $(addsuffix .exe,$(TARGET_NAME))
endif
TARGET := $(BIN_PATH)/$(TARGET_NAME)
TARGET_DEBUG := $(DBG_PATH)/$(TARGET_NAME)
TARGET_LIB := $(LIB_PATH)/lib$(TARGET_NAME).a

# src files & obj files
SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))
OBJ_DEBUG := $(addprefix $(DBG_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

# clean files list
DISTCLEAN_LIST := $(OBJ) \
                  $(OBJ_DEBUG)
CLEAN_LIST := $(TARGET) \
			  $(TARGET_DEBUG) \
			  $(DISTCLEAN_LIST) \
			  $(TARGET_LIB)

# default rule
default: makedir all

# non-phony targets
$(TARGET): $(OBJ)
	$(CC) $(CCFLAGS) -o $@ $(OBJ) $(LIBS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(CCOBJFLAGS) -o $@ $<

$(DBG_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(CCOBJFLAGS) $(DBGFLAGS) -o $@ $<

$(TARGET_DEBUG): $(OBJ_DEBUG)
	$(CC) $(CCFLAGS) $(DBGFLAGS) $(OBJ_DEBUG) -o $@ $(LIBS)

$(TARGET_LIB): $(OBJ)
	$(CC) $(CCFLAGS) -c -o $@ $(OBJ) $(LIBS)

# phony rules
.PHONY: makedir
makedir:
	@mkdir -p $(BIN_PATH) $(OBJ_PATH) $(DBG_PATH) $(LIB_PATH) $(LIB_HEADER_PATH)

.PHONY: all
all: $(TARGET)

.PHONY: debug
debug: $(TARGET_DEBUG)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)
	@rm -rf $(LIB_HEADER_PATH)

.PHONY: lib
lib:
	@make
	@ar ruv $(TARGET_LIB) $(OBJ)
	@ranlib $(TARGET_LIB)
	@cp ./$(SRC_PATH)/*.h ./$(LIB_HEADER_PATH)

.PHONY: distclean
distclean:
	@echo CLEAN $(DISTCLEAN_LIST)
	@rm -f $(DISTCLEAN_LIST)
