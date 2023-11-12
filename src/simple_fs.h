#ifndef simple_fs_h
#define simple_fs_h

#include "accessors.h"
#include "volume_utilites.h"
#include "sfstream.h"

#define DC1 0x11 //Default mark for invalid file
#define DC2 0x12 //Mark for deleted file

template <typename address_t, address_t end, volume_reader<address_t> reader, volume_writer<address_t> writer>
class simple_fs
{
public:
    using this_t = simple_fs<address_t, end, reader, writer>;

    using accessors = volume_accessors<address_t, reader, writer>;

    using utilities = volume_utilities<address_t, reader, writer>;

    using this_fstream = sfstream<address_t, reader, writer>;

    using file_count_t = address_t;

    using file_size_t = address_t;

    static constexpr address_t file_count_address = 0;

    static constexpr address_t file_table_address = sizeof(address_t);

    static constexpr address_t end_address = end;

    struct descriptor
    {
        static constexpr uint8_t namesz_length = 9;

        address_t address = 0;

        file_size_t size = 0;

        char name[descriptor::namesz_length] = { DC1 };

        const bool valid() const
        {
            return name[0] != DC1;
        }

        const bool deleted() const
        {
            return name[0] == DC2;
        }

        operator address_t() const
        {
            return address;
        }

        bool operator==(const descriptor& other) const
        {
            return size == other.size && address == other.address && strncmp(name, other.name, descriptor::namesz_length);
        }
    };

    static constexpr file_size_t overhead(const file_count_t count)
    {
        return sizeof(file_count_t) + (count * sizeof(descriptor));
    }

    void quick_format()
    {
        accessors::template put<file_count_t>(file_count_address, 0);
    }

    void format() 
    {
        utilities::template clear<end>(0);
        quick_format();
    }

    file_count_t count()
    {
        return accessors::template get<file_count_t>(file_count_address);
    }

    file_size_t table_size()
    {
        return count() * sizeof(descriptor);
    }

    file_size_t file_usage()
    {
        file_size_t sum = 0;
        file_count_t count = this->count();
        for (file_count_t i = 0; i < count; i++)
            sum += accessors::template get<file_count_t>(file_table_address + (sizeof(descriptor) * i) + offsetof(descriptor, size));
        return sum;
    }

    file_size_t usage()
    {
        return table_size() + file_usage();
    }

    address_t file_end()
    {
        return end - file_usage();
    }

    descriptor get_descriptor(file_count_t index)
    {
        if (index >= count())
            return descriptor();
        return accessors::template get<descriptor>(file_table_address + (sizeof(descriptor) * index));
    }

    address_t get_descriptor_address(const char* const name)
    {
        file_count_t count = this->count();
        for (address_t address = file_table_address, i = 0; i < count; i++, address += sizeof(descriptor))
        {
            char file_name[descriptor::namesz_length];
            accessors::template get<char[descriptor::namesz_length]>(address + offsetof(descriptor, name), file_name);
            if (strncmp(name, file_name, descriptor::namesz_length) == 0)
                return address;
        }
        return 0;
    }

    descriptor get_descriptor(const char* const name)
    {
        file_count_t count = this->count();
        for (address_t address = file_table_address, i = 0; i < count; i++, address += sizeof(descriptor))
        {
            descriptor file_descriptor = accessors::template get<descriptor>(address);
            if (strncmp(name, file_descriptor.name, descriptor::namesz_length) == 0)
                return file_descriptor;
        }
        return descriptor();
    }

    descriptor get_descriptor(const this_fstream& stream)
    {
        file_count_t count = this->count();
        for (address_t i = 0, address = file_table_address; i < count; i++, address += sizeof(descriptor))
        {
            descriptor file_descriptor = accessors::template get<descriptor>(address);
            if (file_descriptor.address == stream.address())
                return file_descriptor;
        }

        return descriptor();
    }

    this_fstream file_create(const char* const name, file_size_t size, descriptor& out)
    {
        file_count_t count = this->count();

        for (file_count_t i = 0; i < count; i++)
        {
            out = get_descriptor(i);
            if (strncmp(name, out.name, descriptor::namesz_length) == 0)
            {
                out = descriptor();
                return this_fstream();
            }

            if (!out.valid() || !out.deleted())
                continue;

            if (out.size < size)
                continue;

            //Implement using deleted space
            break;
        }

        if (overhead(count + 1) + size > end)
        {
            out = descriptor();
            return this_fstream();
        }

        out.address = file_end() - size + 1;
        out.size = size;
        strncpy(out.name, name, descriptor::namesz_length);
        accessors::template put<descriptor>(file_table_address + table_size(), out);
        accessors::template put<file_count_t>(file_count_address, count + 1);

        for (address_t address = out.address, i = 0; i < out.size; address++, i++)
            writer(address, 0);
        return this_fstream(out.address, out.size);
    }

    this_fstream file_create(const char* const name, file_size_t size)
    {
        descriptor dummy;
        return file_create(name, size, dummy);
    }

    this_fstream file_open(file_count_t index, descriptor& out)
    {
        out = get_descriptor(index);
        if (!out.valid() ||out.deleted())
            return this_fstream();
        return this_fstream(out.address, out.size);
    }

    this_fstream file_open(file_count_t index)
    {
        descriptor dummy;
        return file_open(index, dummy);
    }

    this_fstream file_open(const char* const name, descriptor& out)
    {
        out = get_descriptor(name);
        if (!out.valid() || out.deleted())
            return this_fstream();
        return this_fstream(out.address, out.size);
    }

    this_fstream file_open(const char* const name)
    {
        descriptor dummy;
        return file_open(name, dummy);
    }

    void file_delete(descriptor& file_descriptor)
    {
        if (!file_descriptor.valid() || file_descriptor.deleted())
            return;

        address_t descriptor_address = get_descriptor_address(file_descriptor); //Maybe use one that compares address and not name
        if (descriptor_address == 0)
            return;
        file_descriptor.name[0] = DC2;
        accessors::template put<descriptor>(descriptor_address, file_descriptor);
    }

    void file_rename(descriptor& file_descriptor, const char* const name)
    {
        if (!file_descriptor.valid() || file_descriptor.deleted())
            return;

        address_t descriptor_address = get_descriptor_address(file_descriptor); //Maybe use one that compares address and not name
        if (descriptor_address == 0)
            return;
        strncpy(file_descriptor.name, name, descriptor::namesz_length);
        accessors::template put<descriptor>(descriptor_address, file_descriptor);
    }
};

#endif
