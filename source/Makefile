# Include common Make header
#
include common.mk

SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
TARGET=dbfs
OBJDIR :=.obj

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AT)$(LINK.cpp) $(OBJECTS) -o $@

.cpp.o:
	$(AT)$(COMPILE.cc) $(CFLAGS) $< -o $@

clean:
	$(AT)rm -rf *.o
	$(AT)rm -rf $(TARGET)
