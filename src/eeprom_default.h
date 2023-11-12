#ifndef eeprom_default_h
#define eeprom_default_h

#include "accessors.h"
#include <EEPROM.h>

using eeprom_address_t = uint16_t;

static constexpr eeprom_address_t eeprom_end = E2END;

static uint8_t eeprom_read(eeprom_address_t address)
{
    return EEPROM.read(address);
}

static void eeprom_write(eeprom_address_t address, uint8_t value)
{
#ifdef __USE_WRITE__
    EEPROM.write(address, value);
#else
    EEPROM.update(address, value);
#endif
}

#endif
