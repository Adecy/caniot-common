#include "prng.h"

static uint32_t lfsr32;
static uint32_t lfsr31;

uint32_t shift_lfsr(uint32_t *lfsr, uint32_t poly_mask)
{
    uint32_t feedback = *lfsr & 1;
    *lfsr >>= 1;
    if (feedback == 1) {
        *lfsr ^= poly_mask;
    }
    return *lfsr;
}

void init_lfsrs(void)
{
    /* seed values */
    lfsr32 = INIT_LFSR32;
    lfsr31 = INIT_LFSR31;
}

uint16_t get_random16(void)
{
    shift_lfsr(&lfsr32, PRNG_POLY_MASK_32);
    return (
        shift_lfsr(&lfsr32, PRNG_POLY_MASK_32) ^
        shift_lfsr(&lfsr31, PRNG_POLY_MASK_31)
        ) & 0xFFFF;
}

uint32_t get_random32(void)
{
    return ((uint32_t) get_random16() << 16) | (uint32_t) get_random16();
}