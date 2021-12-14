-include sgxMakefile

CC := clang
CXX := clang++

######## SGX SDK Settings ########

SGX_SDK = /opt/intel/sgxsdk
SGX_MODE = HW
SGX_PRERELEASE ?= 0
SGX_ARCH = x64
SGX_DEBUG ?= 1
SGX_LIBRARY_PATH := $(SGX_SDK)/lib64

OPENSSL_SGX_PATH = $(Enclave_Dir)/OpenSSL_SGX
OPENSSL_SGX_INC = -I$(OPENSSL_SGX_PATH)/include
OPENSSL_LIBS = -lssl -lcrypto

FLATBUFFERS_SGX_PATH = $(Enclave_Dir)/flatbuffers
FLATBUFFERS_SGX_INC = -I$(FLATBUFFERS_SGX_PATH)/include
FLATBUFFERS_SGX_LPATH = $(FLATBUFFERS_SGX_PATH)
FLATBUFFERS_SGX_LIBS = -lflatbuffers

ifeq ($(shell getconf LONG_BIT), 32)
	SGX_ARCH := x86
else ifeq ($(findstring -m32, $(CFLAGS)), -m32)
	SGX_ARCH := x86
endif

 # when you can't win, supress the warnings
SGX_COMMON_FLAGS = -Wno-implicit-exception-spec-mismatch  -Wno-ignored-attributes
ifeq ($(strip $(PERF)), y)
  SGX_COMMON_FLAGS += -DINTRUSIVE_PERF
endif


ifeq ($(SGX_ARCH), x86)
	SGX_COMMON_FLAGS += -m32
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x86/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x86/sgx_edger8r
else
	SGX_COMMON_FLAGS += -m64
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x64/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x64/sgx_edger8r
endif

ifneq ($(strip $(NAGLE)), y)
  SGX_COMMON_FLAGS += -DDISABLE_NAGLE
endif

ifeq ($(SGX_DEBUG), 1)
ifeq ($(SGX_PRERELEASE), 1)
$(error Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!)
endif
endif

SGX_COMMON_FLAGS += -O3

#SGX_COMMON_FLAGS += -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type \
                    #-Waddress -Wsequence-point -Wformat-security \
                    #-Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow \
                    #-Wcast-align -Wcast-qual -Wconversion -Wredundant-decls
SGX_COMMON_CFLAGS := $(SGX_COMMON_FLAGS) -Wno-unused-command-line-argument #-Wjump-misses-init -Wstrict-prototypes
SGX_COMMON_CXXFLAGS := $(SGX_COMMON_FLAGS) -Wnon-virtual-dtor -std=c++11


######## App Settings ########

ifneq ($(SGX_MODE), HW)
	Urts_Library_Name := sgx_urts_sim
	Uae_Service_Library_Name := sgx_uae_service_sim
else
	Urts_Library_Name := sgx_urts
	Uae_Service_Library_Name := sgx_uae_service
endif


App_Include_Paths := -I$(Include_Dir) -I$(App_Dir) -I$(SGX_SDK)/include
App_C_Flags := -fdiagnostics-color=always $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes $(App_Include_Paths) -DSGX_UNTRUSTED

# Three configuration modes - Debug, prerelease, release
#   Debug - Macro DEBUG enabled.
#   Prerelease - Macro NDEBUG and EDEBUG enabled.
#   Release - Macro NDEBUG enabled.
ifeq ($(SGX_DEBUG), 1)
    App_C_Flags += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
    App_C_Flags += -DNDEBUG -DEDEBUG -UDEBUG
else
    App_C_Flags += -DNDEBUG -UEDEBUG -UDEBUG
endif

App_Cpp_Flags := $(App_C_Flags)
App_Link_Flags := $(SGX_COMMON_FLAGS) -lpthread -L$(SGX_LIBRARY_PATH) -l$(Urts_Library_Name) -l$(Uae_Service_Library_Name)
App_Link_Flags += $(App_Link_Locations) $(App_Link_Libs)

App_C_Objects := $(App_C_Files:.c=.o)
App_Cpp_Objects := $(App_Cpp_Files:.cc=.o)

######## Enclave Settings ########

ifneq ($(SGX_MODE), HW)
	Trts_Library_Name := sgx_trts_sim
	Service_Library_Name := sgx_tservice_sim
else
	Trts_Library_Name := sgx_trts
	Service_Library_Name := sgx_tservice
endif
Crypto_Library_Name := sgx_tcrypto




Enclave_Include_Paths := -I$(Include_Dir) -I$(Enclave_Dir) -I$(SGX_SDK)/include -I$(SGX_SDK)/include/libcxx -I$(SGX_SDK)/include/tlibc $(OPENSSL_SGX_INC) $(FLATBUFFERS_SGX_INC)
Enclave_C_Flags := $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden -fpie -ffunction-sections -fdata-sections -fstack-protector-strong $(Enclave_Include_Paths) -fdiagnostics-color=always
Enclave_Cpp_Flags := $(SGX_COMMON_CXXFLAGS) -nostdinc -fvisibility=hidden -fpie -ffunction-sections -fdata-sections -fstack-protector-strong $(Enclave_Include_Paths) -fdiagnostics-color=always -nostdinc++

# Enable the security flags
Enclave_Security_Link_Flags := -Wl,-z,relro,-z,now,-z,noexecstack

# To generate a proper enclave, it is recommended to follow below guideline to link the trusted libraries:
#    1. Link sgx_trts with the `--whole-archive' and `--no-whole-archive' options,
#       so that the whole content of trts is included in the enclave.
#    2. For other libraries, you just need to pull the required symbols.
#       Use `--start-group' and `--end-group' to link these libraries.
# Do NOT move the libraries linked with `--start-group' and `--end-group' within `--whole-archive' and `--no-whole-archive' options.
# Otherwise, you may get some undesirable errors.
Enclave_Link_Flags := $(Enclave_Security_Link_Flags) \
	$(SGX_COMMON_CFLAGS) -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_LIBRARY_PATH) -L$(FLATBUFFERS_SGX_PATH) \
	-Wl,--whole-archive -l$(Trts_Library_Name) -Wl,--no-whole-archive \
	-L$(FLATBUFFERS_SGX_LPATH) $(FLATBUFFERS_SGX_LIBS) \
	-L$(OPENSSL_SGX_PATH) $(OPENSSL_LIBS) \
	-Wl,--start-group -lsgx_tstdc -lsgx_tcxx -lsgx_uprotected_fs -l$(Crypto_Library_Name) -l$(Service_Library_Name) -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0 \
	-Wl,--version-script=$(Enclave_Dir)/enclave.lds \

Enclave_Cpp_Objects := $(Enclave_Cpp_Files:.cc=.o)
Enclave_C_Objects := $(Enclave_C_Files:.c=.o)




ifeq ($(SGX_MODE), HW)
ifeq ($(SGX_DEBUG), 1)
	Build_Mode = HW_DEBUG
else ifeq ($(SGX_PRERELEASE), 1)
	Build_Mode = HW_PRERELEASE
else
	Build_Mode = HW_RELEASE
endif
else
ifeq ($(SGX_DEBUG), 1)
	Build_Mode = SIM_DEBUG
else ifeq ($(SGX_PRERELEASE), 1)
	Build_Mode = SIM_PRERELEASE
else
	Build_Mode = SIM_RELEASE
endif
endif


.PHONY: all target run
all: .config_$(Build_Mode)_$(SGX_ARCH)
	@$(MAKE) target

ifeq ($(Build_Mode), HW_RELEASE)
target:  $(App_Name) $(Enclave_Name)
	@echo "The project has been built in release hardware mode."
	@echo "Please sign the $(Enclave_Name) first with your signing key before you run the $(App_Name) to launch and access the enclave."
	@echo "To sign the enclave use the command:"
	@echo "   $(SGX_ENCLAVE_SIGNER) sign -key <your key> -enclave $(Enclave_Name) -out <$(Signed_Enclave_Name)> -config $(Enclave_Config_File)"
	@echo "You can also sign the enclave using an external signing tool."
	@echo "To build the project in simulation mode set SGX_MODE=SIM. To build the project in prerelease mode set SGX_PRERELEASE=1 and SGX_MODE=HW."


else
target: $(App_Name) $(Signed_Enclave_Name)
ifeq ($(Build_Mode), HW_DEBUG)
	@echo "The project has been built in debug hardware mode."
else ifeq ($(Build_Mode), SIM_DEBUG)
	@echo "The project has been built in debug simulation mode."
else ifeq ($(Build_Mode), HW_PRERELEASE)
	@echo "The project has been built in pre-release hardware mode."
else ifeq ($(Build_Mode), SIM_PRERELEASE)
	@echo "The project has been built in pre-release simulation mode."
else
	@echo "The project has been built in release simulation mode."
endif

endif

run: all
ifneq ($(Build_Mode), HW_RELEASE)
	@echo "RUN	=> $(App_Name) [$(SGX_MODE)|$(SGX_ARCH), OK]"
	@$(CURDIR)/$(App_Name)
endif

.config_$(Build_Mode)_$(SGX_ARCH):
	@rm -f .config_* $(App_Name) $(Enclave_Name) $(Signed_Enclave_Name) $(App_C_Objects) $(App_Dir)/enclave_u.* $(Enclave_C_Objects) $(Enclave_Dir)/enclave_t.*
	@touch .config_$(Build_Mode)_$(SGX_ARCH)

######## App Objects ########

$(App_Dir)/enclave_u.c: $(Enclave_EDL_File)
	@echo "GEN	=> $@"
	@cd $(App_Dir) && $(SGX_EDGER8R) --untrusted ../$(Enclave_EDL_File) --search-path ../$(Enclave_Dir) --search-path $(SGX_SDK)/include

$(App_Dir)/enclave_u.o: $(App_Dir)/enclave_u.c
	@echo "CC 	<= $<"
	@$(CC) $(SGX_COMMON_CFLAGS) $(App_C_Flags) -c $< -o $@

$(App_Dir)/%.o: $(App_Dir)/%.cc
	@echo "CXX	<= $<"
	@$(CXX) $(SGX_COMMON_CXXFLAGS) $(App_Cpp_Flags) -c $< -o $@

$(App_Name): $(App_Dir)/enclave_u.o $(App_Cpp_Objects)
	@echo "LINK	=> $@"
	$(CXX) $^ -o $@ $(App_Link_Flags)

######## Enclave Objects ########

$(Enclave_Dir)/enclave_t.c: $(Enclave_EDL_File)
	@echo "GEN	=> $@"
	@cd $(Enclave_Dir) && $(SGX_EDGER8R) --trusted ../$(Enclave_EDL_File) --search-path ../$(Enclave_Dir) --search-path $(SGX_SDK)/include

$(Enclave_Dir)/enclave_t.o: $(Enclave_Dir)/enclave_t.c
	@echo "CC	<= $<"
	@$(CC) $(Enclave_C_Flags) -c $< -o $@

$(Enclave_Dir)/%.o: $(Enclave_Dir)/%.cc
	@echo "CXX	<= $<"
	@$(CXX) $(SGX_COMMON_CXXFLAGS) $(Enclave_Cpp_Flags) -c $< -o $@

$(Enclave_Dir)/%.o: $(Enclave_Dir)/%.c
	@echo "CC 	<= $<"
	@$(CC) $(SGX_COMMON_CFLAGS) $(Enclave_C_Flags) -c $< -o $@


$(Enclave_Name): $(Enclave_Dir)/enclave_t.o $(Enclave_Cpp_Files:.cc=.o) $(Enclave_C_Files:.c=.o)
	@echo "LINK	=> $@"
	@$(CXX) $^ -o $@ $(Enclave_Link_Flags) $(Enclave_Cpp_Flags)

$(Signed_Enclave_Name): $(Enclave_Name)
	@echo "SIGN => $@"
	@$(SGX_ENCLAVE_SIGNER) sign -key $(Enclave_Dir)/enclave_private.pem -enclave $(Enclave_Name) -ignore-init-sec-error -out $@ -config $(Enclave_Config_File)

.PHONY: fmt
fmt:
	clang-format -i -style=file $(App_Cpp_Files) $(App_Dir)/*.h  $(Enclave_Cpp_Files) $(Enclave_C_Files) $(Enclave_Dir)/*.h

.PHONY: clean
clean:
	@rm -f .config_* $(App_Name) $(Enclave_Name) $(Signed_Enclave_Name) $(App_Cpp_Objects) $(App_C_Objects) $(App_Dir)/enclave_u.* $(Enclave_Cpp_Objects) $(Enclave_C_Objects) $(Enclave_Dir)/enclave_t.* $(Enclave_Dir)/*.i
