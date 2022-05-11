## Makefile for clnt lib
CC := clang
CXX := clang++
INCLUDES := -I. -I./lib
CFLAGS := -fdiagnostics-color=always $(INCLUDES) # -fsanitize=address
LDFLAGS := -L. $(INCLUDES) # -fsanitize=address
LDLIBS += -lssl -lcrypto
LDLIBS += -lstdc++ -lm
LDLIBS += -lflatbuffers
LDLIBS += -lteems

ifneq ($(strip $(DEBUG)), no)
  CFLAGS += -g
endif

ifeq ($(strip $(OPTIM)), no)
  CFLAGS += -O0
else
  CFLAGS += -O3
endif
CXXFLAGS := $(CFLAGS) -Wall -Werror -std=c++14

LIB_SOURCES=$(wildcard lib/*.cc)
LIG_OBJECTS=$(LIB_SOURCES:.cc=.o)
SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:.cc=.o)
TESTS := teems_test latency_test throughput_test close_remote


all: libteems.a $(TESTS)
	@rm -f vgcore.*

-include autodep

libteems.a: $(LIG_OBJECTS)
	ar rcs $@ $^

test: libteems.a $(TESTS)
	@for t in $(TESTS) ; do echo $$t; echo; ./$$t ; echo; done

teems_test: teems_test.o libteems.a
close_remote: close_remote.o libteems.a
latency_test: latency_test.o libteems.a
throughput_test: throughput_test.o libteems.a

#################################

.PHONY: fmt
fmt:
	clang-format -i -style=file $(SOURCES) $(LIB_SOURCES)

.PHONY: clean
clean:
	rm -f *.o *.a vgcore* $(TESTS) lib/*.o

depend : $(SOURCES)
	$(CXX) $(CFLAGS) $(INCLUDES) -MM $(SOURCES) > autodep
	@echo $(SOURCES)
