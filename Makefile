# staticlib target: build static library from tests/module.cpp
# sharedlib target: build shared library from tests/module.cpp
# teststaticlib target: patch micropython to initialize and register the
#   upywraptest module from the static lib in main
# testsharedlib: build micropython and run tests (must use windows-pyd
#   branch for uPy as it has -rdynamic)
#
# Before any lib can be built the MicroPython headers are generated.
# Builds with MICROPY_PY_THREAD=0 to allow finaliser, see gc.c

AR = ar
CP = cp
CXX = g++
MKDIR = mkdir
PYTHON = python3
RM = rm

HASCPP17 = $(shell expr `$(CC) -dumpversion | cut -f1 -d.` \>= 7)
CUR_DIR = $(shell pwd)
MICROPYTHON_DIR ?= ../micropython
MICROPYTHON_PORT_DIR ?= $(MICROPYTHON_DIR)/ports/unix
CPPFLAGS = \
	-Wall -Werror \
 	-I$(MICROPYTHON_DIR) -I$(MICROPYTHON_DIR)/py \
 	-I$(MICROPYTHON_PORT_DIR) -I$(MICROPYTHON_PORT_DIR)/build -I$(MICROPYTHON_PORT_DIR)/variants/standard
ifeq ($(HASCPP17), 1)
	CPPFLAGS += -std=c++17
else
	CPPFLAGS += -std=c++11
endif
MAKEUPY = make -C $(MICROPYTHON_PORT_DIR) BUILD=build
MAKEUPYCROSS = make -C $(MICROPYTHON_DIR)/mpy-cross
UPYFLAGS = MICROPY_PY_BTREE=0 MICROPY_PY_FFI=0 MICROPY_PY_USSL=0 MICROPY_PY_AXTLS=0 MICROPY_FATFS=0 MICROPY_PY_THREAD=0
# For the static library the modules gets built as 'user C module'
# so we need to instruct the build to do that (also see module.c).
UPYFLAGSS = $(UPYFLAGS) USER_C_MODULES=$(CUR_DIR) CFLAGS_EXTRA="-DMODULE_UPYWRAPTEST_ENABLED=1 -DMICROPY_MODULE_BUILTIN_INIT=1"

staticlib:
	$(MAKEUPY) $(UPYFLAGSS) build/genhdr/qstrdefs.generated.h
	$(CXX) $(CPPFLAGS) -c tests/module.cpp -o tests/module_static.o
	$(AR) rcs tests/libupywraptest.a tests/module_static.o

sharedlib:
	$(MAKEUPY) $(UPYFLAGS) build/genhdr/qstrdefs.generated.h
	$(CXX) -fPIC $(CPPFLAGS) -c tests/module.cpp -o tests/module_shared.o
	$(CXX) -shared -o tests/libupywraptest.so tests/module_shared.o
	$(MKDIR) -p ~/.micropython/lib
	$(CP) tests/libupywraptest.so ~/.micropython/lib/upywraptest.so

teststaticlib: staticlib
	$(MAKEUPYCROSS)
	$(MAKEUPY) $(UPYFLAGSS) all
	MICROPY_MICROPYTHON=$(MICROPYTHON_PORT_DIR)/micropython \
	$(PYTHON) $(MICROPYTHON_DIR)/tests/run-tests -d $(CUR_DIR)/tests/py

testsharedlib: sharedlib
	$(MAKEUPYCROSS)
	# Only works with MicroPython windows-pyd branch, which already has the correct linker options
	# so there's no need to add anything here.
	$(MAKEUPY) $(UPYFLAGS)
	MICROPY_MICROPYTHON=$(MICROPYTHON_PORT_DIR)/micropython \
	$(PYTHON) $(MICROPYTHON_DIR)/tests/run-tests --keep-path -d $(CUR_DIR)/tests/py

test: teststaticlib testsharedlib

clean:
	# Just clean everything: we use different flags than the default so to avoid
	# surprises (typically: not all qstrs being detected) make sure everything
	# gets built again after we touched it.
	$(MAKEUPY) clean
	$(RM) -f tests/*.o tests/*.a tests/*.so
