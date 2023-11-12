#ifndef accessors_h
#define accessors_h

#include <stdint.h>

template <typename address_t>
using volume_reader = uint8_t (* const) (address_t);

template <typename address_t>
using volume_writer = void (* const) (address_t, uint8_t);

template <typename address_t, volume_reader<address_t> reader, volume_writer<address_t> writer>
class volume_accessors
{
public:
    using this_t = volume_accessors<address_t, reader, writer>;

    using this_address_t = address_t;

    static volume_reader<address_t> read = reader;

    static void read_bytes(address_t address, void* const buffer, address_t length)
    {
        for (address_t offset = 0; offset < length; offset++)
            ((uint8_t*)buffer)[offset] = read(address + offset);
    }

    template <typename T>
    static void get(address_t address, T& out)
    {
        read_bytes(address, &out, sizeof(out));
    }

    template <typename T>
    static T get(address_t address)
    {
        T out = T();
        get<T>(address, out);
        return out;
    }

    static volume_writer<address_t> write = writer;

    static void write_bytes(address_t address, const void* const buffer, address_t length)
    {
        for (address_t offset = 0; offset < length; offset++)
            write(address + offset, ((uint8_t*)buffer)[offset]);
    }

    template <typename T>
    static void put(address_t address, const T& value)
    {
        write_bytes(address, &value, sizeof(value));
    }
};

#endif
