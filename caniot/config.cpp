#include <config.h>

uint8_t Configuration::checksum() const
{
    uint8_t checksum = 0xFF;
    for (size_t i = 0; i < sizeof(config_t); i++)
    {
        checksum ^= eeprom_read_byte((uint8_t *)data + i);
    }
    return checksum;
}