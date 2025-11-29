
BUNDLER = z-core/zbundler.py
SRC = src/zstr.c
DIST = zstr.h

all: bundle

bundle:
	@echo "Bundling $(DIST)..."
	python3 $(BUNDLER) $(SRC) $(DIST)

init:
	git submodule update --init --recursive
