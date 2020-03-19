#pragma once

#include <cstdio>
#include <string>

namespace oct
{
namespace net
{
namespace tftp
{

class file_wrapper
{
public:
    file_wrapper(const file_wrapper&) = delete;
    file_wrapper& operator=(const file_wrapper&) = delete;

    file_wrapper(const std::string& path, const std::string& mode)
    {
        m_file = std::fopen(path.c_str(), mode.c_str());
    }

    ~file_wrapper()
    {
        if (m_file)
        {
            std::fclose(m_file);
        }
    }

    bool is_open() const
    {
        return m_file != NULL;
    }

protected:
    FILE* get_file()
    {
        return m_file;
    }

private:
    FILE* m_file;
};

class file_reader : public file_wrapper
{
public:
    file_reader(const std::string& path)
        : file_wrapper(path, "rb")
    {
        // noop
    }

    bool read(void* buffer, const std::size_t buffer_size, std::size_t& bytes_read)
    {
        if (!is_open())
        {
            return false;
        }

        bytes_read = std::fread(buffer, 1, buffer_size, get_file());
        if (bytes_read < buffer_size)
        {
            if (std::ferror(get_file()))
            {
                return false;
            }
        }

        return true;
    }
};

class file_writer : public file_wrapper
{
public:
    file_writer(const std::string& path)
        : file_wrapper(path, "wb")
    {
        // noop
    }

    bool write(const void* buffer, const std::size_t bytes_count)
    {
        if (!is_open())
        {
            return false;
        }

        if (std::fwrite(buffer, 1, bytes_count, get_file()) != bytes_count)
        {
            return false;
        }

        return true;
    }
};

} // namespace tftp
} // namespace net
} // namespace oct
