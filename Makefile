# Compiler
CC = gcc

# Directories
SRC_DIR = src
BUILD_DIR = build
DEBUG_DIR = $(BUILD_DIR)/debug
RELEASE_DIR = $(BUILD_DIR)/release

# Files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
DEBUG_OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(DEBUG_DIR)/%.o)
RELEASE_OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(RELEASE_DIR)/%.o)
DEBUG_EXEC = $(DEBUG_DIR)/app
RELEASE_EXEC = $(RELEASE_DIR)/app

# Flags
CFLAGS = -Wall -Wextra -pedantic
DEBUG_FLAGS = -g
RELEASE_FLAGS = -O2

# Default target
all: debug release

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(DEBUG_DIR) $(DEBUG_EXEC)

$(DEBUG_DIR):
	mkdir -p $(DEBUG_DIR)

$(DEBUG_EXEC): $(DEBUG_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Release build
release: CFLAGS += $(RELEASE_FLAGS)
release: $(RELEASE_DIR) $(RELEASE_EXEC)

$(RELEASE_DIR):
	mkdir -p $(RELEASE_DIR)

$(RELEASE_EXEC): $(RELEASE_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean build directories
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all debug release clean

