#pragma once

#include <cstdio>
#include <string>

#include "io.hpp"

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
        m_handle = std::fopen(path.c_str(), mode.c_str());
    }

    ~file_wrapper()
    {
        if (is_open())
        {
            close();
        }
    }

    bool close()
    {
        bool rv = true;

        if (m_handle)
        {
            rv = (std::fclose(m_handle) == 0);
            m_handle = nullptr;
        }

        return rv;
    }

    bool is_open() const
    {
        return m_handle != nullptr;
    }

    FILE* get_handle()
    {
        return m_handle;
    }

private:
    FILE* m_handle;
};

class file_reader : public reader
{
public:
    file_reader(const std::string& path)
        : m_file_wrapper(path, "rb")
    {
        // noop
    }

    bool close() final
    {
        return m_file_wrapper.close();
    }

    bool is_open() const final
    {
        return m_file_wrapper.is_open();
    }

    bool read(void* buffer, const std::size_t buffer_size, std::size_t& bytes_read) final
    {
        if (!m_file_wrapper.is_open())
        {
            return false;
        }

        bytes_read = std::fread(buffer, 1, buffer_size, m_file_wrapper.get_handle());
        if (bytes_read < buffer_size)
        {
            if (std::ferror(m_file_wrapper.get_handle()))
            {
                return false;
            }
        }

        return true;
    }

private:
    file_wrapper m_file_wrapper;
};

class file_writer : public writer
{
public:
    file_writer(const std::string& path)
        : m_file_wrapper(path, "wb")
    {
        // noop
    }

    bool close() final
    {
        return m_file_wrapper.close();
    }

    bool is_open() const final
    {
        return m_file_wrapper.is_open();
    }

    bool write(const void* buffer, const std::size_t bytes_count) final
    {
        if (!m_file_wrapper.is_open())
        {
            return false;
        }

        if (std::fwrite(buffer, 1, bytes_count, m_file_wrapper.get_handle()) != bytes_count)
        {
            return false;
        }

        return true;
    }

private:
    file_wrapper m_file_wrapper;
};

class netascii_reader : public reader
{
public:
    netascii_reader(std::unique_ptr<reader> peer_reader)
        : m_peer_reader(std::move(peer_reader))
    {
        // noop
    }

    bool close() final
    {
        return m_peer_reader->close();
    }

    bool is_open() const final
    {
        return m_peer_reader->is_open();
    }

    bool read(void* buffer, const std::size_t buffer_size, std::size_t& bytes_read) final
    {
        return false;
#if 0
        if (!m_file_wrapper.is_open())
        {
            return false;
        }

        bytes_read = std::fread(buffer, 1, buffer_size, m_file_wrapper.get_handle());
        if (bytes_read < buffer_size)
        {
            if (std::ferror(m_file_wrapper.get_handle()))
            {
                return false;
            }
        }

        return true;
#endif
    }

private:
    std::unique_ptr<reader> m_peer_reader;
};

class netascii_writer : public writer
{
public:
    netascii_writer(std::unique_ptr<writer> peer_writer)
        : m_peer_writer(std::move(peer_writer))
    {
        // noop
    }

    bool close() final
    {
        return m_peer_writer->close();
    }

    bool is_open() const final
    {
        return m_peer_writer->is_open();
    }

    bool write(const void* buffer, const std::size_t bytes_count) final
    {
        return false;
#if 0
        if (!m_file_wrapper.is_open())
        {
            return false;
        }

        if (std::fwrite(buffer, 1, bytes_count, m_file_wrapper.get_handle()) != bytes_count)
        {
            return false;
        }

        return true;
#endif
    }

private:
    std::unique_ptr<writer> m_peer_writer;
};

} // namespace tftp
} // namespace net
} // namespace oct
