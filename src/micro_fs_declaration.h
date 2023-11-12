#ifndef micro_fs_declaration_h
#define micro_fs_declaration_h

#include "accessors.h"
#include "volume_utilites.h"
#include "mfstream_declaration.h"

template <typename address_t, address_t end, volume_reader<address_t> reader, volume_writer<address_t> writer>
class micro_fs
{
public:
    using this_t = micro_fs<address_t, end, reader, writer>;

    using accessors = volume_accessors<address_t, reader, writer>;

    using utilities = volume_utilities<address_t, reader, writer>;

    using this_fstream = mfstream<address_t, end, reader, writer>;

    using file_count_t = address_t;

    using file_size_t = address_t;

    struct descriptor
    {

    };
};

#endif
