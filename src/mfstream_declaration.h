#ifndef mfstream_h
#define mfstream_h

#include "accessors.h"
#include <Stream.h>

template <typename address_t, address_t end, volume_reader<address_t> reader, volume_writer<address_t> writer>
class micro_fs;

template <typename address_t, address_t end, volume_reader<address_t> reader, volume_writer<address_t> writer>
class mfstream : Stream
{
public:
    using base_t = Stream;

    using this_t = mfstream<address_t, end, reader, writer>;

    using this_fs = micro_fs<address_t, end, reader, writer>;

private:
    address_t file_address, file_position, file_size;
};

#endif
