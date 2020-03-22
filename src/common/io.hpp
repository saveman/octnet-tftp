#pragma once

#include <cstdio>
#include <string>

namespace oct
{
namespace net
{
namespace tftp
{

class reader
{
public:
    virtual ~reader() = default;

    virtual bool is_open() const = 0;
    virtual bool read(void* buffer, const std::size_t buffer_size, std::size_t& bytes_read) = 0;
    virtual bool close() = 0;
};

class writer
{
public:
    virtual ~writer() = default;

    virtual bool is_open() const = 0;
    virtual bool write(const void* buffer, const std::size_t bytes_count) = 0;
    virtual bool close() = 0;
};

} // namespace tftp
} // namespace net
} // namespace oct
