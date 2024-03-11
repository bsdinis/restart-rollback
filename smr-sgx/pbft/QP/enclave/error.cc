/***
 * error.c
 *
 * make error handling easier
 */

#include "error.h"
#include "log.h"

void __sgx_perror(const char *func_name, sgx_status_t error) {
    const char *error_msg = "Error not found";

    switch (error) {
        case SGX_SUCCESS:
            error_msg = "No error";
            break;
        case SGX_ERROR_UNEXPECTED:
            error_msg = "Unexpected error ";
            break;
        case SGX_ERROR_INVALID_PARAMETER:
            error_msg = "The parameter is incorrect ";
            break;
        case SGX_ERROR_OUT_OF_MEMORY:
            error_msg =
                "Not enough memory is available to complete this operation ";
            break;
        case SGX_ERROR_ENCLAVE_LOST:
            error_msg =
                "Enclave lost after power transition or used in child process "
                "created by linux:fork() ";
            break;
        case SGX_ERROR_INVALID_STATE:
            error_msg = "SGX API is invoked in incorrect order or state ";
            break;
        case SGX_ERROR_FEATURE_NOT_SUPPORTED:
            error_msg = "Feature is not supported on this platform ";
            break;
        case SGX_ERROR_INVALID_FUNCTION:
            error_msg = "The ecall/ocall index is invalid ";
            break;
        case SGX_ERROR_OUT_OF_TCS:
            error_msg = "The enclave is out of TCS ";
            break;
        case SGX_ERROR_ENCLAVE_CRASHED:
            error_msg = "The enclave is pbfted ";
            break;
        case SGX_ERROR_ECALL_NOT_ALLOWED:
            error_msg =
                "The ECALL is not allowed at this time, e.g. ecall is blocked "
                "by the "
                "dynamic entry table, or nested ecall is not allowed during "
                "initialization ";
            break;
        case SGX_ERROR_OCALL_NOT_ALLOWED:
            error_msg =
                "The OCALL is not allowed at this time, e.g. ocall is not "
                "allowed "
                "during exception handling ";
            break;
        case SGX_ERROR_STACK_OVERRUN:
            error_msg = "The enclave is running out of stack ";
            break;
        case SGX_ERROR_UNDEFINED_SYMBOL:
            error_msg = "The enclave image has undefined symbol. ";
            break;
        case SGX_ERROR_INVALID_ENCLAVE:
            error_msg = "The enclave image is not correct. ";
            break;
        case SGX_ERROR_INVALID_ENCLAVE_ID:
            error_msg = "The enclave id is invalid ";
            break;
        case SGX_ERROR_INVALID_SIGNATURE:
            error_msg = "The signature is invalid ";
            break;
        case SGX_ERROR_NDEBUG_ENCLAVE:
            error_msg =
                "The enclave is signed as product enclave, and can not be "
                "created as "
                "debuggable enclave. ";
            break;
        case SGX_ERROR_OUT_OF_EPC:
            error_msg = "Not enough EPC is available to load the enclave ";
            break;
        case SGX_ERROR_NO_DEVICE:
            error_msg = "Can't open SGX device ";
            break;
        case SGX_ERROR_MEMORY_MAP_CONFLICT:
            error_msg = "Page mapping failed in driver ";
            break;
        case SGX_ERROR_INVALID_METADATA:
            error_msg = "The metadata is incorrect. ";
            break;
        case SGX_ERROR_DEVICE_BUSY:
            error_msg = "Device is busy, mostly EINIT failed. ";
            break;
        case SGX_ERROR_INVALID_VERSION:
            error_msg =
                "Metadata version is inconsistent between uRTS and sgx_sign or "
                "uRTS "
                "is incompatible with current platform. ";
            break;
        case SGX_ERROR_MODE_INCOMPATIBLE:
            error_msg =
                "The target enclave 32/64 bit mode or sim/hw mode is "
                "incompatible "
                "with the mode of current uRTS. ";
            break;
        case SGX_ERROR_ENCLAVE_FILE_ACCESS:
            error_msg = "Can't open enclave file. ";
            break;
        case SGX_ERROR_INVALID_MISC:
            error_msg = "The MiscSelct/MiscMask settings are not correct.";
            break;
        case SGX_ERROR_INVALID_LAUNCH_TOKEN:
            error_msg = "The launch token is not correct.";
            break;
        case SGX_ERROR_MAC_MISMATCH:
            error_msg =
                "Indicates verification error for reports, sealed datas, etc ";
            break;
        case SGX_ERROR_INVALID_ATTRIBUTE:
            error_msg = "The enclave is not authorized ";
            break;
        case SGX_ERROR_INVALID_CPUSVN:
            error_msg = "The cpu svn is beyond platform's cpu svn value ";
            break;
        case SGX_ERROR_INVALID_ISVSVN:
            error_msg = "The isv svn is greater than the enclave's isv svn ";
            break;
        case SGX_ERROR_INVALID_KEYNAME:
            error_msg = "The key name is an unsupported value ";
            break;
        case SGX_ERROR_SERVICE_UNAVAILABLE:
            error_msg =
                "Indicates aesm didn't respond or the requested service is not "
                "supported ";
            break;
        case SGX_ERROR_SERVICE_TIMEOUT:
            error_msg = "The request to aesm timed out ";
            break;
        case SGX_ERROR_AE_INVALID_EPIDBLOB:
            error_msg = "Indicates epid blob verification error ";
            break;
        case SGX_ERROR_SERVICE_INVALID_PRIVILEGE:
            error_msg = "Enclave has no privilege to get launch token ";
            break;
        case SGX_ERROR_EPID_MEMBER_REVOKED:
            error_msg = "The EPID group membership is revoked. ";
            break;
        case SGX_ERROR_UPDATE_NEEDED:
            error_msg = "SGX needs to be updated ";
            break;
        case SGX_ERROR_NETWORK_FAILURE:
            error_msg =
                "Network connecting or proxy setting issue is encountered ";
            break;
        case SGX_ERROR_AE_SESSION_INVALID:
            error_msg = "Session is invalid or ended by server ";
            break;
        case SGX_ERROR_BUSY:
            error_msg = "The requested service is temporarily not availabe ";
            break;
        case SGX_ERROR_MC_NOT_FOUND:
            error_msg =
                "The Monotonic Counter doesn't exist or has been invalided ";
            break;
        case SGX_ERROR_MC_NO_ACCESS_RIGHT:
            error_msg =
                "Caller doesn't have the access right to specified VMC ";
            break;
        case SGX_ERROR_MC_USED_UP:
            error_msg = "Monotonic counters are used out ";
            break;
        case SGX_ERROR_MC_OVER_QUOTA:
            error_msg = "Monotonic counters exceeds quota limitation ";
            break;
        case SGX_ERROR_KDF_MISMATCH:
            error_msg =
                "Key derivation function doesn't match during key exchange ";
            break;
        case SGX_ERROR_UNRECOGNIZED_PLATFORM:
            error_msg =
                "EPID Provisioning failed due to platform not recognized by "
                "backend "
                "server";
            break;
        case SGX_ERROR_UNSUPPORTED_CONFIG:
            error_msg =
                "The config for trigging EPID Provisiong or PSE Provisiong&LTP "
                "is "
                "invalid";
            break;
        case SGX_ERROR_NO_PRIVILEGE:
            error_msg = "Not enough privilege to perform the operation ";
            break;
        case SGX_ERROR_PCL_ENCRYPTED:
            error_msg = "trying to encrypt an already encrypted enclave ";
            break;
        case SGX_ERROR_PCL_NOT_ENCRYPTED:
            error_msg =
                "trying to load a plain enclave using "
                "sgx_create_encrypted_enclave ";
            break;
        case SGX_ERROR_PCL_MAC_MISMATCH:
            error_msg = "section mac result does not match build time mac ";
            break;
        case SGX_ERROR_PCL_SHA_MISMATCH:
            error_msg =
                "Unsealed key MAC does not match MAC of key hardcoded in "
                "enclave "
                "binary ";
            break;
        case SGX_ERROR_PCL_GUID_MISMATCH:
            error_msg =
                "GUID in sealed blob does not match GUID hardcoded in enclave "
                "binary ";
            break;
        case SGX_ERROR_FILE_BAD_STATUS:
            error_msg =
                "The file is in bad status, run sgx_clearerr to try and fix "
                "it ";
            break;
        case SGX_ERROR_FILE_NO_KEY_ID:
            error_msg =
                "The Key ID field is all zeros, can't re-generate the "
                "encryption "
                "key ";
            break;
        case SGX_ERROR_FILE_NAME_MISMATCH:
            error_msg =
                "The current file name is different then the original file "
                "name (not "
                "allowed, substitution attack) ";
            break;
        case SGX_ERROR_FILE_NOT_SGX_FILE:
            error_msg = "The file is not an SGX file ";
            break;
        case SGX_ERROR_FILE_CANT_OPEN_RECOVERY_FILE:
            error_msg =
                "A recovery file can't be opened, so flush operation can't "
                "continue "
                "(only used when no EXXX is returned)  ";
            break;
        case SGX_ERROR_FILE_CANT_WRITE_RECOVERY_FILE:
            error_msg =
                "A recovery file can't be written, so flush operation can't "
                "continue "
                "(only used when no EXXX is returned)  ";
            break;
        case SGX_ERROR_FILE_RECOVERY_NEEDED:
            error_msg =
                "When openeing the file, recovery is needed, but the recovery "
                "process failed ";
            break;
        case SGX_ERROR_FILE_FLUSH_FAILED:
            error_msg =
                "fflush operation (to disk) failed (only used when no EXXX is "
                "returned) ";
            break;
        case SGX_ERROR_FILE_CLOSE_FAILED:
            error_msg =
                "fclose operation (to disk) failed (only used when no EXXX is "
                "returned) ";
            break;
        case SGX_ERROR_UNSUPPORTED_ATT_KEY_ID:
            error_msg =
                "platform quoting infrastructure does not support the key.";
            break;
        case SGX_ERROR_ATT_KEY_CERTIFICATION_FAILURE:
            error_msg = "Failed to generate and certify the attestation key.";
            break;
        case SGX_ERROR_ATT_KEY_UNINITIALIZED:
            error_msg =
                "The platform quoting infrastructure does not have the "
                "attestation "
                "key available to generate quote.";
            break;
        case SGX_ERROR_INVALID_ATT_KEY_CERT_DATA:
            error_msg =
                "TThe data returned by the platform library's "
                "sgx_get_quote_config() "
                "is invalid.";
            break;
        case SGX_ERROR_PLATFORM_CERT_UNAVAILABLE:
            error_msg = "The PCK Cert for the platform is not available.";
            break;
        case SGX_INTERNAL_ERROR_ENCLAVE_CREATE_INTERRUPTED:
            error_msg =
                "The ioctl for enclave_create unexpectedly failed with EINTR. ";
            break;
    }

    ERROR("%s: %s", func_name, error_msg);
}
