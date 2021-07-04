#include "attributes.h"

#include "utils.h"

const uint8_t resolve_attribute(const key_t key, attr_ref_t *const p_attr_ref)
{
    if (ATTR_KEY_SECTION(key) < ARRAY_SIZE(attr_sections))
    {
        const section_t * const section_p = &attr_sections[ATTR_KEY_SECTION(key)];

        if (ATTR_KEY_ATTR(key) < (uint8_t) pgm_read_byte(&section_p->array_size))
        {
            const attribute_t * const attr_array_p = (const attribute_t *) pgm_read_byte(&section_p->array);    // read attr array adress from PROGMEM
            const attribute_t * const attr_p = &attr_array_p[ATTR_KEY_ATTR(key)];                               // get attr address in PEOGMEM
            const uint8_t attr_size = (uint8_t) pgm_read_byte(&attr_p->size);

            if (ATTR_KEY_OFFSET(key) < attr_size)
            {
                const uint8_t attr_offset = ATTR_KEY_OFFSET(key) + (uint8_t) pgm_read_byte(&attr_p->offset);
                const section_option_t option = (section_option_t)(pgm_read_byte(&section_p->options) |
                                                                   (pgm_read_byte(&attr_p->readonly) & READONLY));
                                                                   
                *p_attr_ref = {ATTR_KEY_SECTION(key), option, attr_offset, (uint8_t) MIN(attr_size, 4)};

#if LOG_LEVEL_DBG
                print_attr_ref(p_attr_ref);
#endif

                return CANIOT_OK;
            }
            return CANIOT_EKEYPART;
        }
        return CANIOT_EKEYATTR;
    }
    return CANIOT_EKEYSECTION;
}