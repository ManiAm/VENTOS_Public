#ifndef QDEBUGSTREAM_H
#define QDEBUGSTREAM_H

#include <iostream>
#include <streambuf>
#include <string>

namespace VENTOS {

class QDebugStream : public std::basic_streambuf<char>
{
public:
    QDebugStream(std::ostream &stream, Glib::RefPtr<Gtk::TextBuffer> buff) : m_stream(stream) {
        textBuffer = buff;
        m_old_buf = stream.rdbuf();
        stream.rdbuf(this);
    }

    ~QDebugStream() {
        // output anything that is left
        if (!m_string.empty())
        {
            auto it = textBuffer->end();
            textBuffer->insert(it, m_string);
        }

        m_stream.rdbuf(m_old_buf);
    }

protected:

    virtual int_type overflow(int_type v) {
        if (v == '\n')
        {
            auto it = textBuffer->end();
            textBuffer->insert(it, m_string);

            m_string.erase(m_string.begin(), m_string.end());
        }
        else
            m_string += v;

        return v;
    }

    virtual std::streamsize xsputn(const char *p, std::streamsize n) {
        m_string.append(p, p + n);

        int pos = 0;
        while (pos != (int)std::string::npos)
        {
            pos = m_string.find('\n');
            if (pos != (int)std::string::npos)
            {
                std::string tmp(m_string.begin(), m_string.begin() + pos);

                auto it = textBuffer->end();
                textBuffer->insert(it, tmp);

                m_string.erase(m_string.begin(), m_string.begin() + pos + 1);
            }
        }

        return n;
    }

private:
    std::ostream &m_stream;
    std::streambuf *m_old_buf;
    std::string m_string;
    Glib::RefPtr<Gtk::TextBuffer> textBuffer;
};

}

#endif // QDEBUGSTREAM_H
