#pragma once

#include "io.hpp"
#include "make_unique.hpp"

namespace oct
{
namespace net
{
namespace tftp
{

class netascii_reader : public reader
{
public:
    netascii_reader(std::unique_ptr<reader> peer_reader)
        : m_peer_reader(std::move(peer_reader))
        , m_pending_char(0)
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
        bytes_read = 0;

        auto buffer_ptr = reinterpret_cast<char*>(buffer);
        for (std::size_t i = 0; i < buffer_size; ++i)
        {
            char c;
            int rv = read_char(c);
            if (rv > 0)
            {
                *buffer_ptr++ = c;
                ++bytes_read;
            }
            else if (rv == 0)
            {
                break;
            }
            else
            {
                return false;
            }
        }
        return true;
    }

private:
    int read_char(char& c)
    {
        if (m_pending_char == '\n')
        {
            c = '\n';
            m_pending_char = 0;
            return 1;
        }

        if (m_pending_char == '\r')
        {
            c = '\0';
            m_pending_char = 0;
            return 1;
        }

        std::size_t chars_read = 0;

        if (!m_peer_reader->read(&c, 1, chars_read))
        {
            return false;
        }

        if (chars_read == 1)
        {
            if ((c == '\n') || (c == '\r'))
            {
                m_pending_char = c;
                c = '\r';
            }
            return 1;
        }
        else if (chars_read == 0)
        {
            return 0;
        }
        else
        {
            // should not happen, report error
            return -1;
        }
    }

    std::unique_ptr<reader> m_peer_reader;
    char m_pending_char;
};

class netascii_writer : public writer
{
public:
    netascii_writer(std::unique_ptr<writer> peer_writer)
        : m_peer_writer(std::move(peer_writer))
        , m_pending_cr(false)
    {
        // noop
    }

    bool close() final
    {
        bool rv = true;

        if (m_pending_cr)
        {
            rv &= write_cr();
        }

        rv &= m_peer_writer->close();

        return rv;
    }

    bool is_open() const final
    {
        return m_peer_writer->is_open();
    }

    bool write(const void* buffer, const std::size_t bytes_count) final
    {
        auto buffer_ptr = reinterpret_cast<const char*>(buffer);
        for (std::size_t i = 0; i < bytes_count; ++i)
        {
            char curr_char = *buffer_ptr++;

            if (curr_char == '\r')
            {
                if (m_pending_cr)
                {
                    // cr cr, write the pending one
                    if (!write_cr())
                    {
                        return false;
                    }
                }
                m_pending_cr = true;

                // pending '\r'
                continue;
            }

            if (curr_char == '\n')
            {
                // if CR pending then silently skip it
                m_pending_cr = false;

                // write the '\n'
            }
            else if (curr_char == '\0')
            {
                if (m_pending_cr)
                {
                    // cr 0 -> cr
                    if (!write_cr())
                    {
                        return false;
                    }
                    m_pending_cr = false;

                    // \r written, skip the '\0'
                    continue;
                }

                // write the \0', not related to '\r'
            }

            // write the char
            if (!m_peer_writer->write(&curr_char, 1))
            {
                return false;
            }
        }
        return true;
    }

private:
    bool write_cr()
    {
        const char cr = '\r';
        return m_peer_writer->write(&cr, 1);
    }

    std::unique_ptr<writer> m_peer_writer;
    bool m_pending_cr;
};

} // namespace tftp
} // namespace net
} // namespace oct
