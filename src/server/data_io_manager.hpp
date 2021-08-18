#pragma once

#include <memory>

#include "io.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class data_io_manager
{
public:
    virtual ~data_io_manager() = default;
    virtual std::unique_ptr<reader> create_reader(const std::string& filename) = 0;
    virtual std::unique_ptr<writer> create_writer(const std::string& filename) = 0;
};

} // namespace tftp
} // namespace net
} // namespace oct
