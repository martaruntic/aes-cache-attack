#ifndef AES_LOADER_H
#define AES_LOADER_H

#include "aes.h"

// Loads one of the AES shared libraries at runtime, so the same victim,
// tests and benchmark can run on every implementation.

typedef struct {
    void *handle;
    void (*set_key)(aes128_ctx *, const uint8_t *);
    void (*encrypt)(const aes128_ctx *, const uint8_t *, uint8_t *);
    const char *(*impl_name)(void);
} aes_lib;

// Returns 0 on success, prints the error and returns -1 otherwise
int aes_lib_load(aes_lib *lib, const char *path);
void aes_lib_close(aes_lib *lib);

#endif
