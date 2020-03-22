#pragma once

#include <memory>

#include <asio.hpp>

namespace oct
{
namespace net
{
namespace tftp
{

class connection : public std::enable_shared_from_this<connection>
{
protected:
public:
    virtual ~connection() = default;

    virtual void start() = 0;

    virtual void stop() = 0;

protected:
    template <class derived_T>
    std::shared_ptr<derived_T> shared_from_base()
    {
        return std::static_pointer_cast<derived_T>(shared_from_this());
    }
};

} // namespace tftp
} // namespace net
} // namespace oct
