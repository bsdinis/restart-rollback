## Makefile for clnt lib
CC := clang
CXX := clang++
INCLUDES := -I.
CFLAGS := -fdiagnostics-color=always $(INCLUDES) # -fsanitize=address
LDFLAGS := -L. $(INCLUDES) # -fsanitize=address
LDLIBS += -lssl -lcrypto
LDLIBS += -lstdc++ -lm
LDLIBS += -lflatbuffers
LDLIBS += -lbasicQP

ifneq ($(strip $(DEBUG)), no)
  CFLAGS += -g
endif

ifeq ($(strip $(OPTIM)), no)
  CFLAGS += -O0
else
  CFLAGS += -O3
endif
CXXFLAGS := $(CFLAGS) -Wall -Werror -std=c++14

SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:.cc=.o)
TESTS := qp_test latency_test throughput_test close_remote


all: libbasicQP.a $(TESTS)
	@rm -f vgcore.*

-include autodep

libbasicQP.a: config.o peer.o qp_clnt.o qp_helpers.o qp_network.o ssl_util.o log.h
	ar rcs $@ $^

test: libbasicQP.a $(TESTS)
	@for t in $(TESTS) ; do echo $$t; echo; ./$$t ; echo; done

qp_test: qp_test.o libbasicQP.a
close_remote: close_remote.o libbasicQP.a
latency_test: latency_test.o libbasicQP.a
throughput_test: throughput_test.o libbasicQP.a

#################################

.PHONY: fmt
fmt:
	clang-format -i -style=file $(SOURCES)

.PHONY: clean
clean:
	rm -f *.o *.a vgcore* $(TESTS)

depend : $(SOURCES)
	$(CXX) $(CFLAGS) $(INCLUDES) -MM $(SOURCES) > autodep
	@echo $(SOURCES)
