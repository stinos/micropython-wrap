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
RM = rm
# Note for msys2 builds: it is crucial that the Python version is a cygwin one (e.g. '[GCC 10.2.0] on cygwin')
# not a mingw one (e.g. '[GCC 11.2.0 64 bit (AMD64)] on win32'): when building the user C modules the vpath
# passed to makemoduledefs will be a POSIX path like /C/projects/micropython-wrap but such path is not understood
# by mingw Python builds, only 'native' msys2 i.e. cygwin-style.
PYTHON = python3

HASCPP17 = $(shell expr `$(CXX) -dumpversion | cut -f1 -d.` \>= 7)
HASGCC8 = $(shell expr `$(CXX) -dumpversion | cut -f1 -d.` \>= 8)
CUR_DIR = $(shell pwd)
MICROPYTHON_DIR ?= ../micropython
MPY_CROSS ?= $(MICROPYTHON_DIR)/mpy-cross/mpy-cross
MICROPYTHON_PORT_DIR ?= $(MICROPYTHON_DIR)/ports/unix
CPPFLAGS = \
	-Wall -Werror $(CPPFLAGS_EXTRA)\
 	-I$(MICROPYTHON_DIR) -I$(MICROPYTHON_DIR)/py \
 	-I$(MICROPYTHON_PORT_DIR)
UPYFLAGSUSERCPPMOD = -Wno-missing-field-initializers
ifeq ($(HASGCC8), 1)
	UPYFLAGSUSERCPPMOD += -Wno-cast-function-type -std=c++2a
	CPPFLAGS += -std=c++2a
else ifeq ($(HASCPP17), 1)
	UPYFLAGSUSERCPPMOD += -std=c++17
	CPPFLAGS += -std=c++17
else
	UPYFLAGSUSERCPPMOD += -std=c++11
	CPPFLAGS += -std=c++11
endif
V ?= 0
MAKEOPTS ?= -j4 V=$(V)
MAKEUPY = make -C $(MICROPYTHON_PORT_DIR) $(MAKEOPTS)
UPYFLAGS = MICROPY_PY_BTREE=0 MICROPY_PY_FFI=0 MICROPY_PY_USSL=0 MICROPY_PY_AXTLS=0 MICROPY_FATFS=0 MICROPY_PY_THREAD=0

# Flags instructing MicroPython to build the 'user C module'.
CFLAGS_EXTRA = -DMODULE_UPYWRAPTEST_ENABLED=1
# Slightly different per port unfortunately, just spell out those ports we've tested so far:
# esp32 port has this one enabled already, unix port explicitly disables this but not wrapped in #ifndef
# so supply our own variant to fix that.
ifneq (,$(filter %ports/windows, $(MICROPYTHON_PORT_DIR)))
	CFLAGS_EXTRA += -DMICROPY_MODULE_BUILTIN_INIT=1
else ifneq (,$(filter %ports/unix, $(MICROPYTHON_PORT_DIR)))
	THIS_DIR = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
	VARIANT_DIR = $(THIS_DIR)/build/unixvariant
endif
UPYFLAGSUSERMOD = $(UPYFLAGS) USER_C_MODULES=$(CUR_DIR) CFLAGS_EXTRA="$(CFLAGS_EXTRA)"
ifdef VARIANT_DIR
	UPYFLAGSUSERMOD += VARIANT_DIR=$(VARIANT_DIR)
	CPPFLAGS += -I$(VARIANT_DIR)
else
	CPPFLAGS += -I$(MICROPYTHON_PORT_DIR)/variants/standard
endif

submodules:
	$(MAKEUPY) submodules

$(MPY_CROSS):
	make -C $(MICROPYTHON_DIR)/mpy-cross $(MAKEOPTS)

# This is how we built before MicroPython itself got support for C++ files
# in user modules, leave it here for reference: the principle is to build the C++ code
# into a static library ourselves, then have MicroPython build the user C module,
# only with cmodule.c as source file, while passing the static library into the
# linker so it finds the init_upywraptest function.
# The usercmodule target on the other hand lets MicroPython build both cmodule.c
# and module.cpp in one go. Also see tests/micropython.mk.
staticlib: submodules
	$(MAKEUPY) $(UPYFLAGSUSERMOD) BUILD=build-static build-static/genhdr/qstrdefs.generated.h
	$(MAKEUPY) $(UPYFLAGSUSERMOD) BUILD=build-static build-static/genhdr/root_pointers.h
	$(CXX) $(CPPFLAGS) -I$(MICROPYTHON_PORT_DIR)/build-static -c tests/module.cpp -o tests/module_static.o
	$(AR) rcs tests/libupywraptest.a tests/module_static.o

sharedlib: submodules
	$(MAKEUPY) $(UPYFLAGS) BUILD=build-shared build-shared/genhdr/qstrdefs.generated.h
	$(MAKEUPY) $(UPYFLAGS) BUILD=build-shared build-shared/genhdr/root_pointers.h
	$(CXX) -fPIC $(CPPFLAGS) -I$(MICROPYTHON_PORT_DIR)/build-shared -c tests/module.cpp -o tests/module_shared.o
	$(MKDIR) -p ~/.micropython/lib
ifneq (,$(filter %ports/windows, $(MICROPYTHON_PORT_DIR)))
	$(MAKEUPY) BUILD=build-shared build-shared/micropythoncore.dll
	$(CXX) -shared -o tests/upywraptest.pyd tests/module_shared.o -L$(MICROPYTHON_PORT_DIR)/build-shared -lmicropythoncore
	$(CP) tests/upywraptest.pyd ~/.micropython/lib/upywraptest.pyd
else
	$(CXX) -shared -o tests/libupywraptest.so tests/module_shared.o
	$(CP) tests/libupywraptest.so ~/.micropython/lib/upywraptest.so
endif

usercmodule: $(MPY_CROSS) submodules
	$(MAKEUPY) $(UPYFLAGSUSERMOD) BUILD=build-usercmod UPYWRAP_BUILD_CPPMODULE=1 UPYFLAGSUSERCPPMOD="$(UPYFLAGSUSERCPPMOD)" UPYWRAP_PORT_DIR=$(MICROPYTHON_PORT_DIR) all

teststaticlib: $(MPY_CROSS) staticlib
	$(MAKEUPY) $(UPYFLAGSUSERMOD) BUILD=build-static all
	MICROPY_MICROPYTHON=$(MICROPYTHON_PORT_DIR)/build-static/micropython \
	$(PYTHON) $(MICROPYTHON_DIR)/tests/run-tests.py -d $(CUR_DIR)/tests/py

testsharedlib: $(MPY_CROSS) sharedlib
ifeq (,$(filter %ports/windows, $(MICROPYTHON_PORT_DIR)))
	# Only works with MicroPython windows-pyd branch, which already has the correct linker options
	# so there's no need to add anything here. Not needed for windows since there we built the dll
	# already, it's needed to link the module against, and that automatically creates the .exe as well.
	$(MAKEUPY) $(UPYFLAGS) BUILD=build-shared
endif
	# Note that ~/ intentionally is used without quotes here: when running in an MSYS2 shell there's
	# https://www.msys2.org/docs/filesystem-paths/#environment-variables which would lead to in quoted
	# "~/.micropython" the root (/) being expanded to something like "~c:/msys64/.micropython" which
	# is not the intent. Without quotes bash will just expand it normally.
	MICROPY_MICROPYTHON=$(MICROPYTHON_PORT_DIR)/build-shared/micropython \
	MICROPYPATH=~/.micropython/lib \
	$(PYTHON) $(MICROPYTHON_DIR)/tests/run-tests.py --keep-path -d $(CUR_DIR)/tests/py

testusercmodule: usercmodule
	MICROPY_MICROPYTHON=$(MICROPYTHON_PORT_DIR)/build-usercmod/micropython \
	$(PYTHON) $(MICROPYTHON_DIR)/tests/run-tests.py -d $(CUR_DIR)/tests/py

test: teststaticlib testsharedlib testusercmodule

clean:
	# Just clean everything: we use different flags than the default so to avoid
	# surprises (typically: not all qstrs being detected) make sure everything
	# gets built again after we touched it.
	$(MAKEUPY) BUILD=build-static clean
	$(MAKEUPY) BUILD=build-shared clean
	$(MAKEUPY) BUILD=build-usercmod clean
	$(RM) -f tests/*.o tests/*.a tests/*.so tests/*.pyd ~/.micropython/lib/upywraptest.so
