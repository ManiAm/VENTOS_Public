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
#include <boost/algorithm/string.hpp>


namespace VENTOS {

class debugStream : public std::streambuf
{
private:
    Gtk::TextView *m_TextView;
    std::string syntaxHighlighting = "";
    std::vector<char> buffer_;

    typedef struct tag_def
    {
        std::string text;
        bool caseSensitive;
        std::string fcolor;
        std::string bcolor;
        std::string size;

        bool isBold;
        bool isItalic;
        bool isOblique;
    } tag_def_t;

    std::vector<tag_def_t> allTags;

public:

    explicit debugStream(Gtk::TextView *tw, std::string sh, std::size_t buff_sz = 512) : m_TextView(tw), syntaxHighlighting(sh), buffer_(buff_sz + 1)
    {
        char *base = &buffer_.front();
        setp(base, base + buffer_.size() - 1); // -1 to make overflow() easier

        parseTags();
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

    void parseTags()
    {
        // tags are separated by |
        std::vector<std::string> tags;
        boost::split(tags, syntaxHighlighting, boost::is_any_of("|"));

        // iterate over each tag
        for(std::string tag : tags)
        {
            tag_def_t entry = {};

            // remove leading and trailing spaces from the string
            boost::trim(tag);

            if(tag == "")
                throw std::runtime_error("syntaxHighlightingExpression parameter is not properly formatted. Extra or missing '|' delimiter?");

            // components of each tag are separated by space
            std::vector<std::string> attributes;
            boost::split(attributes, tag, boost::is_any_of("\t "));

            // iterate over each attribute
            for(std::string att : attributes)
            {
                // remove leading and trailing spaces from the string
                boost::trim(att);

                if(att == "")
                    continue;

                // attribute/value are separated by =
                std::vector<std::string> value;
                boost::split(value, att, boost::is_any_of("="));

                if(value.size() != 2)
                    throw std::runtime_error("attribute is not properly formatted in syntaxHighlightingExpression parameter");

                std::string attName = value[0];
                boost::to_lower(attName);

                if(attName == "text")
                    entry.text = extractString(value[1]);
                else if(attName == "casesensitive")
                {
                    std::string val = extractString(value[1]);
                    if(val == "true")
                        entry.caseSensitive = true;
                    else if(val == "false")
                        entry.caseSensitive = false;
                    else
                        throw std::runtime_error("unknown value for caseSensitive attribute");
                }
                else if(attName == "fcolor")
                    entry.fcolor = extractString(value[1]);
                else if(attName == "bcolor")
                    entry.bcolor = extractString(value[1]);
                else if(attName == "size")
                    entry.size = extractString(value[1]);
                else if(attName == "style")
                {
                    // separating font styles
                    std::vector<std::string> fstyles;
                    boost::split(fstyles, extractString(value[1]), boost::is_any_of(","));

                    for(std::string style : fstyles)
                    {
                        // remove leading and trailing spaces from the string
                        boost::trim(style);

                        if(style == "bold")
                            entry.isBold = true;
                        else if(style == "italic")
                            entry.isItalic = true;
                        else if(style == "oblique")
                            entry.isOblique = true;
                        else
                            throw std::runtime_error("unknown style name");
                    }
                }
                else
                    throw std::runtime_error("unknown attribute in syntaxHighlightingExpression parameter");
            }

            allTags.push_back(entry);
        }
    }

    void defineTags()
    {
        if(allTags.empty())
            return;

        // define tags in the text view
        auto textBuffer = m_TextView->get_buffer();
        for(auto &tag : allTags)
        {
            auto textViewTag = textBuffer->create_tag(tag.text + "_tag");

            if(tag.fcolor != "")
                textViewTag->property_foreground() = tag.fcolor;

            if(tag.bcolor != "")
                textViewTag->property_background() = tag.bcolor;

            if(tag.size != "")
                textViewTag->property_font() = tag.size;

            if(tag.isBold)
                textViewTag->property_weight() = PANGO_WEIGHT_BOLD;

            if(tag.isItalic)
                textViewTag->property_style() = Pango::STYLE_ITALIC;

            if(tag.isOblique)
                textViewTag->property_style() = Pango::STYLE_OBLIQUE;
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

        for(auto &tag : allTags)
        {
            Gtk::TextIter match_start;
            Gtk::TextIter match_end;
            while( search_iter.backward_search(tag.text, (tag.caseSensitive) ? Gtk::TEXT_SEARCH_TEXT_ONLY : Gtk::TEXT_SEARCH_CASE_INSENSITIVE, match_start, match_end, limit) )
            {
                textBuffer->apply_tag_by_name(tag.text + "_tag", match_start, match_end);
                search_iter = match_start;
            }
        }
    }

    const std::string extractString(std::string str)
    {
        unsigned first = str.find("'");
        unsigned last = str.find_last_of("'");
        return str.substr (first+1, last-first-1);
    }
};

}

#endif
