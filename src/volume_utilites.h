#ifndef volume_utilities_h
#define volume_utilities_h

#include "accessors.h"
#include <Print.h>
#include <WString.h>

template <typename address_t, volume_reader<address_t> reader, volume_writer<address_t> writer>
struct volume_utilities
{
    using accessors = volume_accessors<address_t, reader, writer>;

    template <address_t end>
    static void clear(const uint8_t value)
    {
        for (address_t address = 0; address <= end; address++)
            accessors::write(address, value);
    }

    template <address_t end, uint8_t width = 16>
    static size_t print_dump(Print& print)
    {
        constexpr size_t markersz_length = (sizeof(address_t) * 2) + 1 + 1;
        constexpr size_t datasz_length = 2 + 1 + 1;
        constexpr size_t line_length = markersz_length + (width * datasz_length);
        constexpr size_t line_count = (end + 1) / width;

        for (size_t line = 0; line < line_count; line++)
        {
            address_t start = line * width;
            String out;
            out.reserve(line_length);

            char marker_format[5];
            sprintf(marker_format, "%%%02dX", sizeof(address_t) * 2);
            char marker[markersz_length];
            sprintf(marker, marker_format, start);
            out += marker;
            out += ' ';

            char ascii[width + 1];
            for (address_t address = start, offset = 0; offset < width; address++, offset++)
            {
                int value = reader(address);
                ascii[offset] = iscntrl(value) || value > 127 ? '.' : (char)value;

                char data[datasz_length];
                sprintf(data, "%02X", value);
                out += data;
                if (offset != width - 1)
                    out += ' ';
            }

            ascii[width] = '\0';
            out += ' ';
            out += ascii;
            print.println(out);
        }
    }

    template <address_t end, uint8_t width = 16>
    static size_t print_dump_ascii(Print& print)
    {
        constexpr size_t markersz_length = (sizeof(address_t) * 2) + 1 + 1;
        constexpr size_t datasz_length = 2 + 1 + 1;
        constexpr size_t line_length = markersz_length + (width * datasz_length) + 1 + width;
        constexpr size_t line_count = (end + 1) / width;

        for (size_t line = 0; line < line_count; line++)
        {
            address_t start = line * width;
            String out;
            out.reserve(line_length);

            char marker_format[5];
            sprintf(marker_format, "%%%02dX", sizeof(address_t) * 2);
            char marker[markersz_length];
            sprintf(marker, marker_format, start);
            out += marker;
            out += ' ';

            for (address_t address = start, offset = 0; offset < width; address++, offset++)
            {
                char data[datasz_length];
                sprintf(data, "%02X", reader(address));
                out += data;
                if (offset != width - 1)
                    out += ' ';
            }
            print.println(out);
        }
    }

    template <address_t end>
    static size_t dump(Print& print)
    {
        for (address_t address = 0; address <= end; address++)
            print.write(reader(address));
    }

    template <typename other_address_t>
    static void dump(volume_writer<other_address_t> other_writer, size_t start, size_t end)
    {
        for (uint64_t address = start; address <= end; address++)
            other_writer(address, reader(address));
    }
};

#endif
