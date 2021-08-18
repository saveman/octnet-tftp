#pragma once

#include <memory>

#include "io.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class mode_io_manager
{
public:
    virtual ~mode_io_manager() = default;
    virtual std::unique_ptr<reader> create_reader(std::unique_ptr<reader> reader, const std::string& mode) = 0;
    virtual std::unique_ptr<writer> create_writer(std::unique_ptr<writer> writer, const std::string& mode) = 0;
};

} // namespace tftp
} // namespace net
} // namespace oct
