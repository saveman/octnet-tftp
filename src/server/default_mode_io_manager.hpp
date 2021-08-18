#pragma once

#include <memory>

#include "io.hpp"
#include "mode_io_manager.hpp"
#include "netascii_io.hpp"
#include "string_utils.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class default_mode_io_manager : public mode_io_manager
{
public:
    std::unique_ptr<reader> create_reader(std::unique_ptr<reader> reader, const std::string& mode) override
    {
        if (equal_ignore_case(mode, "octet"))
        {
            return reader;
        }
        if (equal_ignore_case(mode, "netascii"))
        {
            return stdext::make_unique<netascii_reader>(std::move(reader));
        }
        return nullptr;
    }

    std::unique_ptr<writer> create_writer(std::unique_ptr<writer> writer, const std::string& mode) override
    {
        if (equal_ignore_case(mode, "octet"))
        {
            return writer;
        }
        if (equal_ignore_case(mode, "netascii"))
        {
            return stdext::make_unique<netascii_writer>(std::move(writer));
        }
        return nullptr;
    }
};

} // namespace tftp
} // namespace net
} // namespace oct
