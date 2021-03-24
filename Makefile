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
MPY_CROSS ?= $(MICROPYTHON_DIR)/mpy-cross/mpy-cross
MICROPYTHON_PORT_DIR ?= $(MICROPYTHON_DIR)/ports/unix
CPPFLAGS = \
	-Wall -Werror $(CPPFLAGS_EXTRA)\
 	-I$(MICROPYTHON_DIR) -I$(MICROPYTHON_DIR)/py \
 	-I$(MICROPYTHON_PORT_DIR) -I$(MICROPYTHON_PORT_DIR)/variants/standard
ifeq ($(HASCPP17), 1)
	CPPFLAGS += -std=c++17
else
	CPPFLAGS += -std=c++11
endif
V ?= 0
MAKEOPTS ?= -j4 V=$(V)
MAKEUPY = make -C $(MICROPYTHON_PORT_DIR) $(MAKEOPTS)
UPYFLAGS = MICROPY_PY_BTREE=0 MICROPY_PY_FFI=0 MICROPY_PY_USSL=0 MICROPY_PY_AXTLS=0 MICROPY_FATFS=0 MICROPY_PY_THREAD=0

# Flags instructing MicroPython to build the 'user C module'.
# Slightly different per port unfortunately, just spell out those ports we've tested so far.
CFLAGS_EXTRA = -DMODULE_UPYWRAPTEST_ENABLED=1
ifneq (,$(filter %ports/unix, $(MICROPYTHON_PORT_DIR)))
	CFLAGS_EXTRA += -DMICROPY_MODULE_BUILTIN_INIT=1
endif
UPYFLAGSUSERMOD = $(UPYFLAGS) USER_C_MODULES=$(CUR_DIR) CFLAGS_EXTRA="$(CFLAGS_EXTRA)"

$(MPY_CROSS):
	make -C $(MICROPYTHON_DIR)/mpy-cross $(MAKEOPTS)

# This is how we built before MicroPython itself got support for C++ files
# in user modules, leave it here for reference: the principle is to build the C++ code
# into a static library ourselves, then have MicroPython build the user C module,
# only with cmodule.c as source file, while passing the static library into the
# linker so it finds the init_upywraptest function.
# The usercmodule target on the other hand lets MicroPython build both cmodule.c
# and module.cpp in one go. Also see tests/micropython.mk.
staticlib:
	$(MAKEUPY) $(UPYFLAGSUSERMOD) BUILD=build-static build-static/genhdr/qstrdefs.generated.h
	$(CXX) $(CPPFLAGS) -I$(MICROPYTHON_PORT_DIR)/build-static -c tests/module.cpp -o tests/module_static.o
	$(AR) rcs tests/libupywraptest.a tests/module_static.o

sharedlib:
	$(MAKEUPY) $(UPYFLAGS) BUILD=build-shared build-shared/genhdr/qstrdefs.generated.h
	$(CXX) -fPIC $(CPPFLAGS) -I$(MICROPYTHON_PORT_DIR)/build-shared -c tests/module.cpp -o tests/module_shared.o
	$(CXX) -shared -o tests/libupywraptest.so tests/module_shared.o
	$(MKDIR) -p ~/.micropython/lib
	$(CP) tests/libupywraptest.so ~/.micropython/lib/upywraptest.so

usercmodule: $(MPY_CROSS)
	$(MAKEUPY) $(UPYFLAGSUSERMOD) BUILD=build-usercmod UPYWRAP_BUILD_CPPMODULE=1 UPYWRAP_PORT_DIR=$(MICROPYTHON_PORT_DIR) all

teststaticlib: $(MPY_CROSS) staticlib
	$(MAKEUPY) $(UPYFLAGSUSERMOD) BUILD=build-static all
	MICROPY_MICROPYTHON=$(MICROPYTHON_PORT_DIR)/micropython \
	$(PYTHON) $(MICROPYTHON_DIR)/tests/run-tests.py -d $(CUR_DIR)/tests/py

testsharedlib: $(MPY_CROSS) sharedlib
	# Only works with MicroPython windows-pyd branch, which already has the correct linker options
	# so there's no need to add anything here.
	$(MAKEUPY) $(UPYFLAGS) BUILD=build-shared
	MICROPY_MICROPYTHON=$(MICROPYTHON_PORT_DIR)/micropython \
	$(PYTHON) $(MICROPYTHON_DIR)/tests/run-tests.py --keep-path -d $(CUR_DIR)/tests/py

testusercmodule: usercmodule
	MICROPY_MICROPYTHON=$(MICROPYTHON_PORT_DIR)/micropython \
	$(PYTHON) $(MICROPYTHON_DIR)/tests/run-tests.py -d $(CUR_DIR)/tests/py

test: teststaticlib testsharedlib testusercmodule

clean:
	# Just clean everything: we use different flags than the default so to avoid
	# surprises (typically: not all qstrs being detected) make sure everything
	# gets built again after we touched it.
	$(MAKEUPY) BUILD=build-static clean
	$(MAKEUPY) BUILD=build-shared clean
	$(MAKEUPY) BUILD=build-usercmod clean
	$(RM) -f tests/*.o tests/*.a tests/*.so
