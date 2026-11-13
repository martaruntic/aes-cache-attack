// Speed of each AES implementation in cycles per 16-byte block, to show
// what the countermeasures cost.
//
// Usage: bench lib1.so [lib2.so ...]

#include "aes_loader.h"

#include <stdio.h>
#include <string.h>
#include <x86intrin.h>

#define BLOCKS 2000000
#define RUNS 5

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s lib.so [lib.so ...]\n", argv[0]);
        return 2;
    }

    printf("%-24s %s\n", "implementation", "cycles/block (best of 5)");

    for (int i = 1; i < argc; i++) {
        aes_lib lib;
        aes128_ctx ctx;
        uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                            0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
        uint8_t block[16] = { 0 };

        if (aes_lib_load(&lib, argv[i]) != 0)
            return 1;
        lib.set_key(&ctx, key);

        double best = 1e30;
        for (int run = 0; run < RUNS; run++) {
            uint64_t start = __rdtsc();
            // Each block depends on the previous one, so this measures
            // latency of one encryption, like a server encrypting
            // single requests.
            for (int b = 0; b < BLOCKS; b++)
                lib.encrypt(&ctx, block, block);
            uint64_t end = __rdtsc();
            double per_block = (double)(end - start) / BLOCKS;
            if (per_block < best)
                best = per_block;
        }

        printf("%-24s %.1f\n", lib.impl_name(), best);
        aes_lib_close(&lib);
    }
    return 0;
}
