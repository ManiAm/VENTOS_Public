/****************************************************************************/
/// @file    debugStream.h
/// @author  Mani Amoozadeh <maniam@ucdavis.edu>
/// @author  second author name
/// @date    May 2016
///
/****************************************************************************/
// VENTOS, Vehicular Network Open Simulator; see http:?
// Copyright (C) 2013-2015
/****************************************************************************/
//
// This file is part of VENTOS.
// VENTOS is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef DEBUGSTREAM_H
#define DEBUGSTREAM_H

#include <cassert>
#include <streambuf>
#include <vector>
#include <gtkmm/main.h>

namespace VENTOS {

class debugStream : public std::streambuf
{
private:
    Gtk::TextView *m_TextView;
    std::vector<char> buffer_;

public:

    explicit debugStream(Gtk::TextView *tw, std::size_t buff_sz = 512) : m_TextView(tw), buffer_(buff_sz + 1)
    {
        char *base = &buffer_.front();
        setp(base, base + buffer_.size() - 1); // -1 to make overflow() easier

        defineTags();
    }

    debugStream(const debugStream &);
    debugStream &operator= (const debugStream &);

protected:

    // overflow is called whenever pptr() == epptr()
    virtual int_type overflow(int_type ch)
    {
        if (ch != traits_type::eof())
        {
            // making sure 'pptr' have not passed 'epptr'
            assert(std::less_equal<char *>()(pptr(), epptr()));

            *pptr() = ch;
            pbump(1);  // advancing the write position

            std::ptrdiff_t n = pptr() - pbase();
            pbump(-n);

            // inserting the first n characters pointed by pbase() into the sink_
            std::ostringstream sink_;
            sink_.write(pbase(), n);

            // append sink_ to the text view
            updateTextView(sink_);

            return ch;
        }

        return traits_type::eof();
    }

    // write the current buffered data to the target, even when the buffer isn't full.
    // This could happen when the std::flush manipulator is used on the stream
    virtual int sync()
    {
        std::ptrdiff_t n = pptr() - pbase();
        pbump(-n);

        // inserting the first n characters pointed by pbase() into the sink_
        std::ostringstream sink_;
        sink_.write(pbase(), n);

        // append sink_ to the text view
        updateTextView(sink_);

        return 0;
    }

private:

    void defineTags()
    {
        auto textBuffer = m_TextView->get_buffer();

        {
            auto tag = textBuffer->create_tag("red_text");
            tag->property_foreground() = "red";
        }

        {
            auto tag = textBuffer->create_tag("red_text_bold");
            tag->property_foreground() = "red";
            tag->property_weight() = PANGO_WEIGHT_BOLD;
        }

        {
            auto tag = textBuffer->create_tag("green_text");
            tag->property_foreground() = "green";
        }
    }

    void updateTextView(std::ostringstream & sink_)
    {
        auto textBuffer = m_TextView->get_buffer();
        auto iter = textBuffer->end();
        textBuffer->insert(iter, sink_.str());

        addTextFormatting();

        // get an iterator to the end of textBuffer
        auto iter2 = textBuffer->end();
        // move the iterator to the beginning of last line
        iter2.set_line_offset(0);

        // move 'last_line' mark to the beginning of the last line
        auto mark = textBuffer->get_mark("last_line");
        textBuffer->move_mark(mark, iter2);

        // scroll the last inserted line into view
        m_TextView->scroll_to(mark);
    }

    void addTextFormatting()
    {
        auto textBuffer = m_TextView->get_buffer();
        auto end_iter = textBuffer->end();
        auto search_iter = end_iter;

        auto mark = textBuffer->get_mark("last_line");
        Gtk::TextIter limit = textBuffer->get_iter_at_mark(mark);

        {
            Gtk::TextIter match_start;
            Gtk::TextIter match_end;
            while( search_iter.backward_search(">>>>", Gtk::TEXT_SEARCH_CASE_INSENSITIVE, match_start, match_end, limit) )
            {
                textBuffer->apply_tag_by_name("green_text", match_start, match_end);
                search_iter = match_start;
            }
        }

        {
            Gtk::TextIter match_start;
            Gtk::TextIter match_end;
            while( search_iter.backward_search("ERROR", Gtk::TEXT_SEARCH_CASE_INSENSITIVE, match_start, match_end, limit) )
            {
                textBuffer->apply_tag_by_name("red_text", match_start, match_end);
                search_iter = match_start;
            }
        }
    }
};

}

#endif
