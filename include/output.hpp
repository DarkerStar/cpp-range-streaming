/* 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STD_RANGEIO_output_
#define STD_RANGEIO_output_

#include <initializer_list>
#include <iosfwd>
#include <utility>

#include "stream-formatting-saver.hpp"

namespace std {
namespace rangeio_detail {

template <typename Range, typename Iterator = decltype(begin(declval<Range&>()))>
struct range_writer
{
  explicit range_writer(Range&& r) :
    range_{forward<Range>(r)}
  {
    next = begin(range_);
  }
  
  Range range_;
  size_t count = 0;
  Iterator next;
};

template <typename Range, typename Iterator, typename CharT, typename Traits>
auto operator<<(basic_ostream<CharT, Traits>& out, range_writer<Range, Iterator>& p) ->
  basic_ostream<CharT, Traits>&
{
  p.count = 0;
  p.next = begin(p.range_);
  
  {
    auto const formatting = stream_formatting_saver<CharT, Traits>{out};
    
    while ((p.next != end(p.range_)) && static_cast<bool>(out))
    {
      formatting.restore();
      
      if ((out << *(p.next)))
      {
        ++p.count;
        ++p.next;
      }
    }
    
    if (!p.count && static_cast<bool>(out))
    {
      for (auto n = out.width(); n && static_cast<bool>(out); --n)
        out.put(out.fill());
    }
  }
  
  out.width(0);
  
  return out;
}

template <typename Range, typename Iterator, typename CharT, typename Traits>
auto operator<<(basic_ostream<CharT, Traits>& out, range_writer<Range, Iterator>&& p) ->
  basic_ostream<CharT, Traits>&
{
  return out << p;
}

template <typename Range, typename Delim, typename Iterator = decltype(begin(declval<Range&>()))>
struct range_writer_delimited
{
  explicit range_writer_delimited(Range&& r, Delim&& d) :
    range_{forward<Range>(r)},
    delim_{forward<Delim>(d)}
  {
    next = begin(range_);
  }
  
  Range range_;
  Delim delim_;
  size_t count = 0;
  Iterator next;
};

template <typename Range, typename Delim, typename Iterator, typename CharT, typename Traits>
auto operator<<(basic_ostream<CharT, Traits>& out, range_writer_delimited<Range, Delim, Iterator>& p) ->
  basic_ostream<CharT, Traits>&
{
  p.count = 0;
  p.next = begin(p.range_);
  
  {
    auto const formatting = stream_formatting_saver<CharT, Traits>{out};
    
    while ((p.next != end(p.range_)) && static_cast<bool>(out))
    {
      formatting.restore();
      
      if ((out << *(p.next)))
      {
        ++p.count;
        ++p.next;
        
        if (p.next != end(p.range_))
          out << p.delim_;
      }
    }
    
    if (!p.count && static_cast<bool>(out))
    {
      for (auto n = out.width(); n && static_cast<bool>(out); --n)
        out.put(out.fill());
    }
  }
  
  out.width(0);
  
  return out;
}

template <typename Range, typename Delim, typename Iterator, typename CharT, typename Traits>
auto operator<<(basic_ostream<CharT, Traits>& out, range_writer_delimited<Range, Delim, Iterator>&& p) ->
  basic_ostream<CharT, Traits>&
{
  return out << p;
}

} // namespace rangeio_detail

template <typename Range>
auto write_all(Range&& r) ->
  rangeio_detail::range_writer<Range&&>
{
  return {forward<Range>(r)};
}

template <typename T>
auto write_all(std::initializer_list<T>&& r) ->
  rangeio_detail::range_writer<std::initializer_list<T>&&>
{
  return {forward<std::initializer_list<T>>(r)};
}

template <typename Range, typename Delim>
auto write_all(Range&& r, Delim&& d) ->
  rangeio_detail::range_writer_delimited<Range&&, Delim&&>
{
  return {forward<Range>(r), forward<Delim>(d)};
}

template <typename T, typename Delim>
auto write_all(std::initializer_list<T>&& r, Delim&& d) ->
  rangeio_detail::range_writer_delimited<std::initializer_list<T>&&, Delim&&>
{
  return {forward<std::initializer_list<T>>(r), forward<Delim>(d)};
}

} // namespace std

#endif  // STD_RANGEIO_output_
