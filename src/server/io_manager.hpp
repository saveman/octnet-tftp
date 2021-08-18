#pragma once

#include <memory>

#include "data_io_manager.hpp"
#include "io.hpp"
#include "mode_io_manager.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class io_manager
{
public:
    io_manager(data_io_manager& data_io_manager, mode_io_manager& mode_io_manager)
        : m_data_io_manager(data_io_manager)
        , m_mode_io_manager(mode_io_manager)
    {
        // noop
    }

    std::unique_ptr<reader> create_reader(const std::string& filename, const std::string& mode)
    {
        auto reader = m_data_io_manager.create_reader(filename);
        if (reader)
        {
            reader = m_mode_io_manager.create_reader(std::move(reader), mode);
        }
        return reader;
    }

    std::unique_ptr<writer> create_writer(const std::string& filename, const std::string& mode)
    {
        auto writer = m_data_io_manager.create_writer(filename);
        if (writer)
        {
            writer = m_mode_io_manager.create_writer(std::move(writer), mode);
        }
        return writer;
    }

private:
    data_io_manager& m_data_io_manager;
    mode_io_manager& m_mode_io_manager;
};

} // namespace tftp
} // namespace net
} // namespace oct
