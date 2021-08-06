#include <config.h>

uint8_t Configuration::checksum() const
{
    uint8_t checksum = 0XFF;
    for (uint16_t addr = 0; addr < sizeof(config_t); addr++)
    {
        checksum ^= eeprom_read_byte((uint8_t *)addr);
    }
    return checksum;
}