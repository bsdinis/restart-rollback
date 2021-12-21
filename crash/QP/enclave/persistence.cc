/**
 * persitence.cc
 */

#include "persistence.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include "crypto_helpers.h"
#include "log.h"

namespace {
size_t g_offset = 0;
sgx_aes_gcm_128bit_tag_t g_previous_mac;

class PersistentBlock {
   public:
    PersistentBlock(size_t slot_n, int64_t account, int64_t amount, int64_t to)
        : m_slot_n(slot_n), m_account(account), m_amount(amount), m_to(to) {}

    ssize_t encrypt(uint8_t *ptr) {
        if (paxos_sgx::crash::crypto::encrypt(
                ptr, this, sizeof(PersistentBlock), m_slot_n, &m_mac) == -1) {
            return -1;
        }

        memcpy(g_previous_mac, m_mac, sizeof(sgx_aes_gcm_128bit_tag_t));
        return paxos_sgx::crash::crypto::padded_size(sizeof(PersistentBlock));
    }

   private:
    size_t m_slot_n;
    int64_t m_account;
    int64_t m_amount;
    int64_t m_to;
    sgx_aes_gcm_128bit_tag_t m_mac;
};

}  // namespace

extern void *g_persistent_array;
extern size_t g_persistent_array_size;

namespace paxos_sgx {
namespace crash {
namespace persistence {

int log_accepted(size_t slot_n, int64_t account, int64_t amount, int64_t to) {
    if (g_offset == 0) {
        memset(g_previous_mac, 0, sizeof(sgx_aes_gcm_128bit_tag_t));
    } else if (g_offset >= g_persistent_array_size) {
        ERROR("Out of space for persistent log: rolling around");
        g_offset = 0;
    }

    PersistentBlock block(slot_n, account, amount, to);

    ssize_t ret = block.encrypt((uint8_t *)g_persistent_array + g_offset);
    if (ret == -1) {
        ERROR("failed to encrypt block for slot %zu", slot_n);
        return -1;
    }

    g_offset += ret;
    return 0;
}

}  // namespace persistence
}  // namespace crash
}  // namespace paxos_sgx
