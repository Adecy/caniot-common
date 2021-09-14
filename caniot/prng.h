#ifndef _AVRTOS_PRDM_H
#define _AVRTOS_PRDM_H

#include <avr/io.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*___________________________________________________________________________*/

/* random number generation using lfsr */
// - https://www.maximintegrated.com/en/design/technical-documents/app-notes/4/4400.html
// - https://en.wikipedia.org/wiki/Linear-feedback_shift_register

#define PRNG_POLY_MASK_32 0xB4BCD35C
#define PRNG_POLY_MASK_31 0x7A5BC2E3

/* this init numbers must be different for each devices, because if
 * all devices were restarted and after a broadcast telemetry request 
 * we should ensure that every delay is different to minimize collisions
 */
#define MAGIC_NUMBER  (((uint32_t) __FIRMWARE_VERSION__ << 16) | \
    ((uint32_t) __DEVICE_TYPE__ << 8) | ((uint32_t) __DEVICE_ID__))

#define INIT_LFSR32     0xAEDC1FF ^ MAGIC_NUMBER
#define INIT_LFSR31     0x12345678 ^ MAGIC_NUMBER

uint32_t shift_lfsr(uint32_t *lfsr, uint32_t poly_mask);

void init_lfsrs(void);

uint16_t get_random16(void);

uint32_t get_random32(void);

/*___________________________________________________________________________*/

#ifdef __cplusplus
}
#endif

#endif