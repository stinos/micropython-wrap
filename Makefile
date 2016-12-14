# lib target: build static library from tests/module.cpp; first build micropython
#             since we need all qstrings generated
# test target: patch micropython to initialize and register the
#              upywraptest module from the static libe in main
# build with MICROPY_PY_THREAD=0 to allow finaliser, see gc.c

CUR_DIR = $(shell pwd)
MICROPYTHON_DIR = ../micropython

lib:
	cd $(MICROPYTHON_DIR)/unix && make axtls && make MICROPY_PY_THREAD=0
	g++ -std=c++11 -I$(MICROPYTHON_DIR) -I$(MICROPYTHON_DIR)/py -I$(MICROPYTHON_DIR)/unix -I$(MICROPYTHON_DIR)/unix/build/ -DMICROPY_PY_THREAD=0 -c tests/module.cpp -o tests/module.o
	ar rcs tests/libupywraptests.a tests/module.o

test: lib
	cd $(MICROPYTHON_DIR)/unix && patch -i $(CUR_DIR)/main.diff
	cd $(MICROPYTHON_DIR)/unix && make MICROPY_PY_THREAD=0 LDFLAGS_MOD="$(CUR_DIR)/tests/libupywraptests.a ../lib/axtls/_stage/libaxtls.a -ldl -lstdc++ -lffi"
	cd $(MICROPYTHON_DIR)/unix && patch -R -i $(CUR_DIR)/main.diff
	cd $(MICROPYTHON_DIR)/tests && python3 run-tests -d $(CUR_DIR)/tests/py

all: test

clean:
	rm tests/*.o tests/*.a $(MICROPYTHON_DIR)/unix/micropython
