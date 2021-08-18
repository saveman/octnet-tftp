#pragma once

#include <iostream>

#include "../server/data_io_manager.hpp"
#include "file_io.hpp"
#include "make_unique.hpp"
#include "netascii_io.hpp"
#include "string_utils.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class file_data_io_manager : public data_io_manager
{
public:
    file_data_io_manager(const std::string& root_path)
        : data_io_manager()
        , m_root_path(root_path)
    {
        // noop
    }

    std::unique_ptr<reader> create_reader(const std::string& filename) final
    {
        if (filename.find("..") != std::string::npos)
        {
            std::cerr << "Invalid filename: " << filename << std::endl;
            return nullptr;
        }

        auto path = m_root_path;
        path += '/';
        path += filename;

        auto reader = stdext::make_unique<file_reader>(path);
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

    std::unique_ptr<writer> create_writer(const std::string& filename) final
    {
        if (filename.find("..") != std::string::npos)
        {
            std::cerr << "Invalid filename: " << filename << std::endl;
            return nullptr;
        }

        auto path = m_root_path;
        path += '/';
        path += filename;

        auto writer = stdext::make_unique<file_writer>(path);
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
    std::string m_root_path;
};

} // namespace tftp
} // namespace net
} // namespace oct
