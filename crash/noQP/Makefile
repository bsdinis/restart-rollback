## Makefile for noQP
CC := clang
CXX := clang++
INCLUDES := -I.
CFLAGS := -fdiagnostics-color=always $(INCLUDES)
LDFLAGS := -L. $(INCLUDES)
LDLIBS += -lssl -lcrypto
LDLIBS += -lssl -lcrypto
LDLIBS += -lstdc++ -lm
LDLIBS += -lflatbuffers

ifneq ($(strip $(DEBUG)), no)
  CFLAGS += -g
endif

ifeq ($(strip $(OPTIM)), no)
  CFLAGS += -O0
else
  CFLAGS += -O3
endif

ifeq ($(strip $(PERF)), y)
  CFLAGS += -DINTRUSIVE_PERF
endif

CXXFLAGS := $(CFLAGS) -Wall -Werror -std=c++14

SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:.cc=.o)
TARGET=crashs


all: $(TARGET)
	@rm -f vgcore.*

-include autodep

crashs: $(OBJECTS)

#################################

.PHONY: fmt
fmt:
	clang-format -i -style=file $(SOURCES)

.PHONY: clean
clean:
	rm -f *.o *.a vgcore* $(TARGET)

depend : $(SOURCES)
	$(CXX) $(CFLAGS) $(INCLUDES) -MM $(SOURCES) > autodep
	@echo $(SOURCES)
