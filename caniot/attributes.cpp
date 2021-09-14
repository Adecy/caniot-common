#include "attributes.h"

#include "misc.h"

extern constexpr const struct Attributes::attribute_t Attributes::identification_attr[];
extern constexpr const struct Attributes::attribute_t Attributes::system_attr[];
extern constexpr const struct Attributes::attribute_t Attributes::config_attr[];
extern constexpr const struct Attributes::section_t Attributes::attr_sections[];

const uint8_t Attributes::resolve(const key_t key, attr_ref_t *const p_attr_ref)
{
    if (ATTR_KEY_SECTION(key) >= ARRAY_SIZE(attr_sections)) {
        return CANIOT_EKEYSECTION;
    }

    const section_t * const section = &attr_sections[ATTR_KEY_SECTION(key)];
    if (ATTR_KEY_ATTR(key) >= (uint8_t)pgm_read_byte(&section->array_size)) {
        return CANIOT_EKEYATTR;
    }

    /* read attr array adress from PROGMEM */
    const attribute_t* const attr_array_p =
        (const attribute_t*)pgm_read_word(&section->array);

    /* get attr address in PROGMEM */
    const attribute_t* const attr = &attr_array_p[ATTR_KEY_ATTR(key)];
    const uint8_t attr_size = (uint8_t)pgm_read_byte(&attr->size);
    if (ATTR_KEY_OFFSET(key) >= attr_size) {
        return CANIOT_EKEYPART;
    }

    const uint8_t attr_offset =
        ATTR_KEY_OFFSET(key) +
        (uint8_t)pgm_read_byte(&attr->offset);
        
    const section_option_t option = (section_option_t)(
        pgm_read_byte(&section->options) |
        (pgm_read_byte(&attr->readonly) & READONLY));

    *p_attr_ref = {
        ATTR_KEY_SECTION(key),
        option,
        attr_offset,
        (uint8_t)MIN(attr_size, 4)
    };

#if LOG_LEVEL_DBG
        print_attr_ref(p_attr_ref);
#endif

    return CANIOT_OK;
}


/*___________________________________________________________________________*/

// todo shorten this switch with an array of pointers, get rid of nullptr check
void* Attributes::get_section_address(const uint8_t section)
{
    can_device* const p_instance = can_device::get_instance();

    switch (section) {
        case ATTR_IDENTIFICATION:
            return &p_instance->identification;

        case ATTR_SYSTEM:
            return &p_instance->system;

        case ATTR_CONFIG:
            return &p_instance->config.data;

        // case ATTR_SCHEDULE:
        //     return &((config_t*) &p_instance->config.data)->schedule;

        default:
            return nullptr;
    }
}

const uint8_t Attributes::read(const attr_ref_t* const attr, value_t* const p_value)
{
    uint8_t err = CANIOT_ENULL;
    if (attr != nullptr) {
        const void* p = (void*)(
            (uint16_t)get_section_address(attr->section) + attr->offset);
        if (attr->options & RAM) // priority for RAM
        {
            memcpy(p_value, p, attr->read_size);
        } else if (attr->options & PROGMEMORY) {
            memcpy_P(p_value, p, attr->read_size);
        } else if (attr->options & EEPROM) {
            eeprom_read_block(p_value, p, attr->read_size);
        } else {
            return CANIOT_ENIMPL;
        }
        err = CANIOT_OK;
    }
    return err;
}

const uint8_t Attributes::write(const attr_ref_t* const attr, const value_t value)
{
    if (attr == nullptr) {
        return CANIOT_ENULL;
    }

    if (attr->options & READONLY) {
        return CANIOT_EREADONLY;
    }

    void* p = (void*)((uint16_t)get_section_address(attr->section) + attr->offset);
    if (attr->options & RAM) {
        memcpy(p, (void*)&value, attr->read_size);
    } else if (attr->options & PROGMEMORY) {
        return CANIOT_EREADONLY;
    } else if (attr->options & EEPROM) {
        eeprom_write_block(p, (void*)&value, attr->read_size);
    } else {
        return CANIOT_ENIMPL;
    }
    return CANIOT_OK;
}