## Makefile for clnt lib
CC := clang
CXX := clang++
INCLUDES := -I. -I./lib -I/usr/include/hiredis
#ifneq ($(strip $(OPENSSL_AES)), yes)
# INCLUDES +=  -I$(HOME)/opt/intel-ipp-crypto/include
#endif
CFLAGS := -fdiagnostics-color=always $(INCLUDES) -fsanitize=address
LDFLAGS := -L. $(INCLUDES) -fsanitize=address

#ifneq ($(strip $(OPENSSL_AES)), yes)
# LDFLAGS += -L$(HOME)/opt/intel-ipp-crypto/lib -rpath $(HOME)/opt/intel-ipp-crypto/lib
# LDLIBS += -lippcp
#endif
LDLIBS += -lssl -lcrypto
LDLIBS += -lstdc++ -lm
LDLIBS += -lflatbuffers
LDLIBS += -lhiredis
LDLIBS += -ls3
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
CXXFLAGS := $(CXXFLAGS) -Wno-unused-function

LIB_SOURCES=$(wildcard lib/*.cc)
LIB_HEADERS=$(wildcard lib/*.h)
LIB_OBJECTS=$(LIB_SOURCES:.cc=.o)
TEST_SOURCES=$(wildcard tests/*.cc)
TEST_OBJECTS=$(SOURCES:.cc=.o)
SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLES := close_remote
TESTS := tests/teems_test tests/latency_test tests/throughput_test tests/metadata_test tests/untrusted_test tests/lru_test

#ifeq ($(strip $(OPENSSL_AES)), no)
  AES_MODE = -DOPENSSL_AES
  $(warning ***** building OpenSSL version ******)
#else
#  AES_MODE = -DINTEL_AES
#  $(info ***** building Intel IPP version ******)
#endif
CXXFLAGS += $(AES_MODE)
INCLUDES += $(AES_MODE)

all: libteems.a $(TESTS) $(EXECUTABLES)
	@rm -f vgcore.*

-include autodep

libteems.a: $(LIB_OBJECTS) $(LIB_HEADERS)
	ar rcs $@ $^

close_remote: close_remote.o libteems.a
tests/latency_test: tests/latency_test.o libteems.a
tests/lru_test: tests/lru_test.o libteems.a
tests/metadata_test: tests/metadata_test.o libteems.a
tests/teems_test: tests/teems_test.o libteems.a
tests/throughput_test: tests/throughput_test.o libteems.a
tests/untrusted_test: tests/untrusted_test.o libteems.a

#################################

.PHONY: fmt
fmt:
	clang-format -i -style=file $(SOURCES) $(LIB_SOURCES) $(TEST_SOURCES) $(LIB_HEADERS)

.PHONY: clean
clean:
	rm -f *.o *.a vgcore* $(TESTS) $(EXECUTABLES) lib/*.o tests/*.o
	rm -rf teems_ustor.*

depend : $(SOURCES)
	$(CXX) $(CFLAGS) $(INCLUDES) -MM $(SOURCES) $(LIB_SOURCES) $(TEST_SOURCES) > autodep
	@echo $(SOURCES)
