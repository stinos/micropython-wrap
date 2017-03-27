# staticlib target: build static library from tests/module.cpp
# sharedlib target: build shared library from tests/module.cpp
# teststaticlib target: patch micropython to initialize and register the
#   upywraptest module from the static lib in main
# testsharedlib: build micropython and run tests (must use windows-pyd
#   branch for uPy as it has -rdynamic)
#
# Before any lib can be built the MicroPython headers are generated.
# Builds with MICROPY_PY_THREAD=0 to allow finaliser, see gc.c

CUR_DIR = $(shell pwd)
MICROPYTHON_DIR = ../micropython
CPPFLAGS = -Wall -Werror -std=c++11 -I$(MICROPYTHON_DIR) -I$(MICROPYTHON_DIR)/py -I$(MICROPYTHON_DIR)/unix -I$(MICROPYTHON_DIR)/unix/build -DMICROPY_PY_THREAD=0
MAKEUPY = make -C $(MICROPYTHON_DIR)/unix
UPYFLAGS = MICROPY_PY_BTREE=0 MICROPY_PY_FFI=0 MICROPY_PY_USSL=0 MICROPY_PY_AXTLS=0 MICROPY_FATFS=0 MICROPY_PY_THREAD=0

upyhdr:
	$(MAKEUPY) $(UPYFLAGS) build/genhdr/qstrdefs.generated.h

staticlib: upyhdr
	g++ $(CPPFLAGS) -c tests/module.cpp -o tests/module_static.o
	ar rcs tests/libupywraptest.a tests/module_static.o

sharedlib: upyhdr
	g++ -fPIC $(CPPFLAGS) -c tests/module.cpp -o tests/module_shared.o
	g++ -shared -o tests/libupywraptest.so tests/module_shared.o
	mkdir -p ~/.micropython/lib
	cp tests/libupywraptest.so ~/.micropython/lib/upywraptest.so

teststaticlib: staticlib
	cd $(MICROPYTHON_DIR)/unix && patch -i $(CUR_DIR)/main.diff
	$(MAKEUPY) $(UPYFLAGS) LDFLAGS_MOD="$(CUR_DIR)/tests/libupywraptest.a -ldl -lstdc++"
	cd $(MICROPYTHON_DIR)/unix && patch -R -i $(CUR_DIR)/main.diff
	cd $(MICROPYTHON_DIR)/tests && python3 run-tests -d $(CUR_DIR)/tests/py

testsharedlib: sharedlib
	# Only works with MicroPython windows-pyd branch, which already has the correct linker options
	# so there's no need to add anything here.
	$(MAKEUPY) $(UPYFLAGS)
	cd $(MICROPYTHON_DIR)/tests && python3 run-tests --keep-path -d $(CUR_DIR)/tests/py

test: teststaticlib testsharedlib

clean:
	rm tests/*.o tests/*.a tests/*.so $(MICROPYTHON_DIR)/unix/micropython
