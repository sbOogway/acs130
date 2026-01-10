CC = gcc
CFLAGS = -Wall -Wextra -fPIC -Iinclude $(shell pkg-config --cflags libmodbus)
LDFLAGS = $(shell pkg-config --libs libmodbus)

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

LIB_SRC = $(SRC_DIR)/acs_130_modbus.c
MAIN_SRC = $(SRC_DIR)/main.c

LIB_OBJ = $(BUILD_DIR)/acs_130_modbus.o
MAIN_OBJ = $(BUILD_DIR)/main.o

LIB_STATIC = libacs_130_modbus.a
LIB_SHARED = libacs_130_modbus.so
MAIN_TARGET = $(BUILD_DIR)/main

.PHONY: all static shared clean directories

all: directories $(MAIN_TARGET)

directories:
	@mkdir -p $(BUILD_DIR)

static: directories $(LIB_STATIC)

shared: directories $(LIB_SHARED)

$(MAIN_TARGET): $(MAIN_OBJ) $(LIB_STATIC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/main_shared: $(MAIN_OBJ) $(LIB_SHARED)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJ) -L. -lacs_130_modbus $(LDFLAGS)

$(LIB_STATIC): $(LIB_OBJ)
	ar rcs $@ $^

$(LIB_SHARED): $(LIB_OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(LIB_STATIC) $(LIB_SHARED) $(BUILD_DIR)/main $(BUILD_DIR)/main_shared
