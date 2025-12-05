# MBASIC WebAssembly Build
# Builds mbasicc as a WebAssembly module for browser execution

# Emscripten compiler
CXX := em++
CC := emcc

# Source directories
MBASIC_DIR := ../mbasicc
MBASIC_SRC := $(MBASIC_DIR)/src
MBASIC_INC := $(MBASIC_DIR)/include

# Compiler flags
CXXFLAGS := -std=c++17 -O2
CXXFLAGS += -I$(MBASIC_INC) -Iinclude

# Emscripten-specific flags
EMFLAGS := -s WASM=1
EMFLAGS += -s MODULARIZE=1
EMFLAGS += -s EXPORT_ES6=1
EMFLAGS += -s EXPORT_NAME="createMBasic"
EMFLAGS += -s ALLOW_MEMORY_GROWTH=1
EMFLAGS += -s ENVIRONMENT='web'
EMFLAGS += -s NO_EXIT_RUNTIME=1
EMFLAGS += -s ASYNCIFY=1
EMFLAGS += -s 'ASYNCIFY_IMPORTS=["js_input","js_inkey"]'
EMFLAGS += --bind

# Core mbasic library sources
# Note: console_io.cpp is needed because ConsoleIO vtable is referenced
MBASIC_CORE_SRCS := \
	$(MBASIC_SRC)/value.cpp \
	$(MBASIC_SRC)/tokens.cpp \
	$(MBASIC_SRC)/lexer.cpp \
	$(MBASIC_SRC)/error.cpp \
	$(MBASIC_SRC)/ast.cpp \
	$(MBASIC_SRC)/parser.cpp \
	$(MBASIC_SRC)/runtime.cpp \
	$(MBASIC_SRC)/interpreter.cpp \
	$(MBASIC_SRC)/console_io.cpp

# Web-specific sources
WEB_SRCS := \
	src/wasm_io.cpp \
	src/wasm_filesystem.cpp \
	src/wasm_bindings.cpp

# All sources
ALL_SRCS := $(MBASIC_CORE_SRCS) $(WEB_SRCS)

# Output
OUTPUT := web/mbasic.js

.PHONY: all clean serve

all: $(OUTPUT)

$(OUTPUT): $(ALL_SRCS)
	$(CXX) $(CXXFLAGS) $(EMFLAGS) -o $@ $(ALL_SRCS)

clean:
	rm -f web/mbasic.js web/mbasic.wasm

# Simple development server
serve: all
	cd web && python3 -m http.server 8080
