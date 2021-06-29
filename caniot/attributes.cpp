#include "attributes.h"

const uint8_t Attributes::resolve_attribute(const key_t key, attr_ref_t *const p_attr_ref)
{
    if (ATTR_KEY_SECTION(key) < ARRAY_SIZE(attributes_sections))
    {
        const section_t *section_p = &attributes_sections[ATTR_KEY_SECTION(key)];
        uint8_t attr_index = 0;
        uint8_t offset = 0;

        for (uint_fast8_t i = 0; i < ARRAY_SIZE(attributes); i++)
        {
            const uint8_t section = pgm_read_byte(&section_p->section);
            if (pgm_read_byte(&attributes[i].section) == section)
            {
                const uint8_t attr_size = pgm_read_byte(&attributes[i].size);
                if (attr_index == ATTR_KEY_ATTR(key))
                {
                    const uint8_t attr_offset = ATTR_KEY_PART(key) << 2;
                    if (attr_offset < attr_size)
                    {
                        const section_option_t option = (section_option_t)(pgm_read_byte(&section_p->options) |
                                                                           (pgm_read_byte(&attributes[i].readonly) & (1 << 3)));
                        *p_attr_ref = {
                            section,
                            option,
                            (uint8_t)(offset + attr_offset),
                            (uint8_t)(attr_size - attr_offset)};

                        return CANIOT_OK;
                    }
                    else
                    {
                        return CANIOT_EKEYPART;
                    }
                }
                else
                {
                    offset += attr_size;
                    attr_index++;
                }
            }
        }
        return CANIOT_EKEYATTR;
    }
    return CANIOT_EKEYSECTION;
}

void *Attributes::get_section_address(const uint8_t section)
{
    switch (section)
    {
    case ATTR_IDENTIFICATION:
        return can_device::get_instance()->p_identification;

    case ATTR_SYSTEM:
        return can_device::get_instance()->p_system;

    case ATTR_CONFIG:
        return can_device::get_instance()->p_config;

    case ATTR_SCHEDULES:
        return can_device::get_instance()->p_schedules;

    default:
        return nullptr;
    }
}

const uint8_t Attributes::read_attribute(const key_t key, value_t *const p_value)
{
    attr_ref_t attr_ref;

    const uint8_t err = resolve_attribute(key, &attr_ref);
    if (err == CANIOT_OK)
    {
        if (attr_ref.options & READONLY)
        {
            return CANIOT_EREADONLY;
        }

        const void *p = get_section_address(attr_ref.section);
        
        if ((p != nullptr) || (attr_ref.options & EEPROM)) // nullptr is a valid EEPROM address
        {
            p = (void *)((uint16_t)p + attr_ref.offset);

            if (attr_ref.options & RAM)
            {
                memcpy(p_value, p, attr_ref.size);
            }
            else if (attr_ref.options & PROGMEMORY)
            {
                memcpy_P(p_value, p, attr_ref.size);
            }
            else if (attr_ref.options & EEPROM)
            {
                eeprom_read_block(p_value, p, attr_ref.size);
            }
            else
            {
                return CANIOT_ENIMPL;
            }

            return CANIOT_OK;
        }
        else
        {
            return CANIOT_ENIMPL;
        }
    }
    return err;
}

const uint8_t Attributes::write_attribute(const key_t key, const value_t value)
{
    attr_ref_t attr_ref;

    const uint8_t err = resolve_attribute(key, &attr_ref);
    if (err == CANIOT_OK)
    {
        void *p = get_section_address(attr_ref.section);
        if (p != nullptr)
        {
            p = (void *)((uint16_t)p + attr_ref.offset);

            if (attr_ref.options & RAM)
            {
                memcpy(p, (void *)&value, attr_ref.size);
            }
            else if (attr_ref.options & PROGMEMORY)
            {
                return CANIOT_EREADONLY;
            }
            else if (attr_ref.options & EEPROM)
            {
                eeprom_write_block(p, (void *)&value, attr_ref.size);
            }
            else
            {
                return CANIOT_ENIMPL;
            }

            return CANIOT_OK;
        }
        else
        {
            return CANIOT_ENIMPL;
        }
    }
    return err;
}