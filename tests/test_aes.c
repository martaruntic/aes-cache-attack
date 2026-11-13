// Checks that every AES implementation is correct:
//  - FIPS-197 Appendix C.1 test vector
//  - 10000 random keys and blocks, compared against the first library
//
// Usage: test_aes lib1.so [lib2.so ...]

#include "aes_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t fips_key[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
};
static const uint8_t fips_plain[16] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
};
static const uint8_t fips_cipher[16] = {
    0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30,
    0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a,
};

#define RANDOM_TESTS 10000

static void random_bytes(uint8_t *buf, int len)
{
    for (int i = 0; i < len; i++)
        buf[i] = (uint8_t)(rand() & 0xff);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s lib.so [lib.so ...]\n", argv[0]);
        return 2;
    }

    int lib_count = argc - 1;
    aes_lib *libs = calloc(lib_count, sizeof(aes_lib));
    int failed = 0;

    for (int i = 0; i < lib_count; i++) {
        if (aes_lib_load(&libs[i], argv[i + 1]) != 0)
            return 1;
    }

    for (int i = 0; i < lib_count; i++) {
        aes128_ctx ctx;
        uint8_t out[16];

        libs[i].set_key(&ctx, fips_key);
        libs[i].encrypt(&ctx, fips_plain, out);

        int ok = memcmp(out, fips_cipher, 16) == 0;
        printf("[%s] %-22s FIPS-197 C.1\n", ok ? "PASS" : "FAIL", libs[i].impl_name());
        if (!ok)
            failed++;
    }

    srand(1234);
    for (int i = 1; i < lib_count; i++) {
        int mismatches = 0;
        for (int t = 0; t < RANDOM_TESTS; t++) {
            uint8_t key[16], plain[16], expected[16], got[16];
            aes128_ctx ref_ctx, ctx;

            random_bytes(key, 16);
            random_bytes(plain, 16);
            libs[0].set_key(&ref_ctx, key);
            libs[0].encrypt(&ref_ctx, plain, expected);
            libs[i].set_key(&ctx, key);
            libs[i].encrypt(&ctx, plain, got);
            if (memcmp(expected, got, 16) != 0)
                mismatches++;
        }
        int ok = mismatches == 0;
        printf("[%s] %-22s matches %s on %d random blocks\n", ok ? "PASS" : "FAIL",
               libs[i].impl_name(), libs[0].impl_name(), RANDOM_TESTS);
        if (!ok)
            failed++;
    }

    for (int i = 0; i < lib_count; i++)
        aes_lib_close(&libs[i]);
    free(libs);

    return failed ? 1 : 0;
}
