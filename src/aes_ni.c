// AES-128 with the AES-NI instructions. The S-box is computed in hardware,
// there are no tables in memory, so there is nothing for a cache attack
// to look at. Used as the baseline for the overhead comparison.

#include "aes.h"

#include <wmmintrin.h>

static __m128i expand_step(__m128i key, __m128i assist)
{
    assist = _mm_shuffle_epi32(assist, 0xff);
    key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
    key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
    key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
    return _mm_xor_si128(key, assist);
}

#define EXPAND(i, rcon) \
    rk[i] = expand_step(rk[i - 1], _mm_aeskeygenassist_si128(rk[i - 1], rcon))

void aes128_set_key(aes128_ctx *ctx, const uint8_t key[AES128_KEY_SIZE])
{
    __m128i *rk = (__m128i *)ctx->round_keys;

    rk[0] = _mm_loadu_si128((const __m128i *)key);
    EXPAND(1, 0x01);
    EXPAND(2, 0x02);
    EXPAND(3, 0x04);
    EXPAND(4, 0x08);
    EXPAND(5, 0x10);
    EXPAND(6, 0x20);
    EXPAND(7, 0x40);
    EXPAND(8, 0x80);
    EXPAND(9, 0x1b);
    EXPAND(10, 0x36);
}

void aes128_encrypt(const aes128_ctx *ctx, const uint8_t in[AES_BLOCK_SIZE],
                    uint8_t out[AES_BLOCK_SIZE])
{
    const __m128i *rk = (const __m128i *)ctx->round_keys;
    __m128i s = _mm_loadu_si128((const __m128i *)in);

    s = _mm_xor_si128(s, rk[0]);
    for (int i = 1; i < 10; i++)
        s = _mm_aesenc_si128(s, rk[i]);
    s = _mm_aesenclast_si128(s, rk[10]);

    _mm_storeu_si128((__m128i *)out, s);
}

const char *aes128_impl_name(void)
{
    return "AES-NI";
}
