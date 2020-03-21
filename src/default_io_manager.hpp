#pragma once

#include <iostream>

#include "file_io.hpp"
#include "io_manager.hpp"
#include "make_unique.hpp"
#include "netascii_io.hpp"
#include "string_utils.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class default_io_manager : public io_manager
{
public:
    default_io_manager(const std::string& root_path)
        : io_manager()
        , m_root_path(root_path)
    {
        // noop
    }

    std::unique_ptr<reader> create_reader(const std::string& filename, const std::string& mode) final
    {
        if (filename.find("..") != std::string::npos)
        {
            std::cerr << "Invalid filename: " << filename << std::endl;
            return nullptr;
        }

        auto path = m_root_path;
        path += '/';
        path += filename;

        auto reader = open_reader(path, mode);
        if (!reader)
        {
            std::cerr << "Open failed: " << path << std::endl;
        }
        else if (!reader->is_open())
        {
            std::cerr << "File not found: " << path << std::endl;
        }
        return reader;
    }

    std::unique_ptr<writer> create_writer(const std::string& filename, const std::string& mode) final
    {
        if (filename.find("..") != std::string::npos)
        {
            std::cerr << "Invalid filename: " << filename << std::endl;
            return nullptr;
        }

        auto path = m_root_path;
        path += '/';
        path += filename;

        auto writer = open_writer(path, mode);
        if (!writer)
        {
            std::cerr << "Open failed: " << path << std::endl;
        }
        else if (!writer->is_open())
        {
            std::cerr << "File not found: " << path << std::endl;
        }

        return writer;
    }

private:
    std::unique_ptr<reader> open_reader(const std::string& path, const std::string& mode)
    {
        if (equal_ignore_case(mode, "octet"))
        {
            return stdext::make_unique<file_reader>(path);
        }
        if (equal_ignore_case(mode, "netascii"))
        {
            return stdext::make_unique<netascii_reader>(stdext::make_unique<file_reader>(path));
        }
        return nullptr;
    }

    std::unique_ptr<writer> open_writer(const std::string& path, const std::string& mode)
    {
        if (equal_ignore_case(mode, "octet"))
        {
            return stdext::make_unique<file_writer>(path);
        }
        if (equal_ignore_case(mode, "netascii"))
        {
            return stdext::make_unique<netascii_writer>(stdext::make_unique<file_writer>(path));
        }
        return nullptr;
    }

    const std::string& m_root_path;
};

} // namespace tftp
} // namespace net
} // namespace oct
