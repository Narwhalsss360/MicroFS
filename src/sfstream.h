#ifndef sfstream_h
#define sfstream_h

#include "accessors.h"
#include <Stream.h>

template <typename address_t, volume_reader<address_t> reader, volume_writer<address_t> writer>
class sfstream : public Stream
{
public:
    using base_t = Stream;

    using this_t = sfstream<address_t, reader, writer>;

    sfstream()
        : file_address(0), file_position(0), file_size(0)
    {
    }

    sfstream(address_t address, address_t size, address_t position = 0)
        : file_address(address), file_position(position), file_size(size)
    {
    }

    int availableForWrite()
    {
        if (!valid())
            return 0;
        return file_size - 1 - file_position;
    }

    size_t write(uint8_t value)
    {
        if (availableForWrite() == 0)
            return 0;
        writer(file_address + file_position, value);
        if (file_position < file_size - 1)
            file_position++;
        return 1;
    }

    int available()
    {
        if (!valid())
            return 0;
        return file_size - 1 - file_position;
    }

    int read()
    {
        if (available() == 0)
            return EOF;
        return reader(file_address + file_position++);
    }

    int peek()
    {
        if (available() == 0)
            return EOF;
        return reader(file_address + file_position);
    }

    address_t seek(address_t offset, address_t origin = 0)
    {
        file_position = origin + offset;
        if (file_position >= file_size)
            file_position = file_size - 1;
        return file_position;
    }

    address_t address() const
    {
        return file_address;
    }

    address_t position() const
    {
        return file_position;
    }

    address_t size() const
    {
        return file_size;
    }

    bool valid() const
    {
        return file_size != 0;
    }

    operator bool() const
    {
        return valid();
    }

    ~sfstream()
    {
    }

private:
    address_t file_address, file_position, file_size;
};

#endif
