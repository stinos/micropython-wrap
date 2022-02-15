EXAMPLE_MOD_DIR := $(USERMOD_DIR)
SRC_USERMOD += $(EXAMPLE_MOD_DIR)/cmodule.c

ifeq ($(UPYWRAP_BUILD_CPPMODULE), 1)
	SRC_USERMOD_CXX += $(EXAMPLE_MOD_DIR)/module.cpp
	ifneq (,$(filter %ports/esp32, $(UPYWRAP_PORT_DIR)))
		CXXFLAGS_USERMOD += -I$(BOARD_DIR) -Wno-error=cpp -Wno-error=sign-compare
	endif
	# Use CXXFLAGS_MOD instead of CXXFLAGS_USERMOD because the unix MakeFile
	# adds its own -std=c++... to CXXFLAGS_MOD if it's not there yet, and then
	# adds it to CXXFLAGS_USERMOD. But we want to specify our own -std=c++ version.
	CXXFLAGS_MOD += $(UPYFLAGSUSERCPPMOD)
	LDFLAGS_USERMOD += -lstdc++
else
	LDFLAGS_USERMOD += $(EXAMPLE_MOD_DIR)/libupywraptest.a -lstdc++
endif
