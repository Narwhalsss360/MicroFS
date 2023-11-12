#ifndef minimal_fs_h
#define minimal_fs_h

#include "accessors.h"
#include "sfstream.h"

template <typename address_t, volume_reader<address_t> reader, volume_writer<address_t> writer>
class minimal_fs
{
public:
    using this_t = minimal_fs<address_t, reader, writer>;

    using accessors = volume_accessors<address_t, reader, writer>;

    using this_fstream = sfstream<address_t, reader, writer>;

    using file_size_t = uint8_t;

    using file_count_t = uint8_t;

    static constexpr address_t file_count_address = 0;

    static constexpr address_t file_table_address = sizeof(file_count_t);

    constexpr file_size_t overhead(file_count_t count)
    {
        return sizeof(file_count_t) + sizeof(file_size_t) * count;
    }

    file_count_t count() const
    {
        return reader(file_count_address);
    }

    template <file_size_t size>
    void index_file()
    {
        file_count_t count = this->count() + 1;
        writer(file_count_address, count);
        writer(count, size);
    }

    template <typename T>
    void index_file()
    {
        index_file<sizeof(T)>();
    }

    template <file_count_t count, typename array_size_t>
    void index(const array_size_t (&sizes)[count])
    {
        writer(file_count_address, count);
        for (file_count_t i = 0; i < count; i++)
            writer(file_table_address + i, (file_size_t)sizes[i]);
    }

    template <file_count_t index>
    address_t address()
    {
        file_count_t count = this->count();
        if (index >= count)
            return 0;

        address_t sum = count + file_table_address;
        for (address_t address = file_table_address; address <= index; address++)
            sum += reader(address);
        return sum;
    }

    template <file_count_t index>
    file_count_t size() const
    {
        if (index >= count())
            return 0;
        return reader(file_table_address + index);
    }

    address_t end() const
    {
        file_count_t count = this->count();

        address_t sum = count + file_table_address;
        for (address_t address = file_table_address; address <= count; address++)
            sum += reader(address);
        return sum;
    }

    template <file_count_t index>
    this_fstream stream()
    {
        return this_fstream(address<index>(), size<index>());
    }
};

#endif
