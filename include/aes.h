#ifndef AES_H
#define AES_H

#include <stdint.h>

// Common interface of the AES-128 implementations in this repo. Each one is
// built as a shared library and loaded with dlopen, so the tests (and later
// the victim) can run on any of them.
//
//   libaes_ttable.so   T-table AES (vulnerable to cache attacks)

#define AES_BLOCK_SIZE 16
#define AES128_KEY_SIZE 16

typedef struct {
    // 11 round keys. The T-table code uses them as 44 32-bit words,
    // the AES-NI code as 11 128-bit registers.
    uint8_t round_keys[176] __attribute__((aligned(16)));
} aes128_ctx;

void aes128_set_key(aes128_ctx *ctx, const uint8_t key[AES128_KEY_SIZE]);
void aes128_encrypt(const aes128_ctx *ctx, const uint8_t in[AES_BLOCK_SIZE],
                    uint8_t out[AES_BLOCK_SIZE]);

// Name of the implementation, printed by the victim and the tests
const char *aes128_impl_name(void);

#endif
