###############################################################
#
# Microsoft(R) MSPAL Common Makefile
#
###############################################################

CXX := clang++-3.7

# Set platform type. Don't use += operator as it will stick in extra spaces.
#
PLATFORM:=$(PLATFORM)$(shell grep -qi suse /etc/os-release && echo -n "suse")
PLATFORM:=$(PLATFORM)$(shell grep -qi rhel /etc/os-release && echo -n "rhel")
PLATFORM:=$(PLATFORM)$(shell grep -qi ubuntu /etc/os-release && echo -n "ubuntu")
PLATFORM_VERSION:=$(PLATFORM_VERSION)$(shell grep -qi 16 /etc/os-release && echo -n "16")
PLATFORM_VERSION:=$(PLATFORM_VERSION)$(shell grep -qi 15 /etc/os-release && echo -n "15")

# To beef up the warnings we subscribe to everything (-Weverything)
# by default. Then we ignore the things which aren't really applicable
# to us, or might take a bit more work to fix across the board.
#
IGNORED_WARNINGS = -Wno-nested-anon-types \
				   -Wno-vla-extension \
				   -Wno-vla \
				   -Wno-c++98-compat \
				   -Wno-c++98-compat-pedantic \
	   			   -Wno-gnu-anonymous-struct \
				   -Wno-zero-length-array \
				   -Wno-variadic-macros \
				   -Wno-c99-extensions \
				   -Wno-padded \
				   -Wno-packed \
				   -Wno-old-style-cast \
				   -Wno-disabled-macro-expansion \
				   -Wno-sign-conversion \
				   -Wno-cast-align \
				   -Wno-weak-vtables \
				   -Wno-global-constructors \
				   -Wno-exit-time-destructors \
				   -Wno-used-but-marked-unused \
				   -Wno-covered-switch-default \
				   -Wno-format-pedantic \
				   -Wno-reserved-id-macro \
				   -Wignored-attributes

INCLUDES=-I.
CFLAGS=-Wall $(IGNORED_WARNINGS) $(INCLUDES) $(shell pkg-config fuse --cflags) -std=c++14

# Dynamic libraries:
#   - libpthread is needed for pthreads support.
#   - librt is needed for async IO (aio_*) APIs.
#   - libdl is needed for dynamic linking.
#   - lsysdb is needed for using the sybase API's
#
LDLIBS += -lpthread -lrt -ldl $(shell pkg-config fuse --libs) -lsybdb

ifeq ($(PLATFORM),$(filter $(PLATFORM),rhel suse))
	LDLIBS += -lc++abi
	INCLUDES += -I/usr/include/c++/v1/
	CFLAGS += -stdlib=libc++
	LDFLAGS += -stdlib=libc++
endif

LDFLAGS += $(LDLIBS)
