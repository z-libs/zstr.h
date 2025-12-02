# Configuration.
BUNDLER = z-core/zbundler.py
SRC_CORE = src/zstr.c
DIST_HEADER = zstr.h

# Standard Lua (Default: 5.4).
LUA_STD_VER = lua5.4
LUA_STD_INC = /usr/include/$(LUA_STD_VER)
LUA_STD_LIB = $(LUA_STD_VER)

# LuaJIT (Manual command settings).
LUA_JIT_INC = /usr/include/luajit-2.1
LUA_JIT_LIB = luajit-5.1

# Source/Output settings
LUA_SRC = bindings/lua/zstr_module.c
LUA_OUT = zstr.so

all: bundle lua

bundle:
	@echo "Bundling $(DIST_HEADER)..."
	@mkdir -p include
	python3 $(BUNDLER) $(SRC_CORE) $(DIST_HEADER) 

lua: LUA_CURRENT_INC = $(LUA_STD_INC)
lua: LUA_CURRENT_LIB = $(LUA_STD_LIB)
lua: build_msg = "Building Standard Lua 5.4 binding..."
lua: $(DIST_HEADER) $(LUA_SRC) build_shared

luajit: LUA_CURRENT_INC = $(LUA_JIT_INC)
luajit: LUA_CURRENT_LIB = $(LUA_JIT_LIB)
luajit: build_msg = "Building LuaJIT binding..."
luajit: $(DIST_HEADER) $(LUA_SRC) build_shared

build_shared:
	@echo $(build_msg)
	gcc -O3 -shared -fPIC -o $(LUA_OUT) $(LUA_SRC) -I. -I$(LUA_CURRENT_INC) -l$(LUA_CURRENT_LIB)

clean:
	rm -f $(LUA_OUT)

init:
	git submodule update --init --recursive

.PHONY: all bundle lua luajit clean init build_shared
