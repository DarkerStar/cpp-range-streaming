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

#ifndef STD_RANGEIO_stream_formatting_saver_
#define STD_RANGEIO_stream_formatting_saver_

#include <iosfwd>

namespace std {
namespace rangeio_detail {

/* 
 * This is a helper class that stores the formatting state of a stream, and
 * restores that state on demand. It is used to keep the formatting consistent
 * for each item in the range being read or written.
 */
template <typename CharT, typename Traits>
struct stream_formatting_saver
{
  explicit stream_formatting_saver(basic_ios<CharT, Traits>& s) :
    stream_{s},
    flags_{s.flags()},
    width_{s.width()},
    precision_{s.precision()},
    fill_{s.fill()}
  {}
  
  void restore() const
  {
    stream_.flags(flags_);
    stream_.fill(fill_);
    stream_.precision(precision_);
    stream_.width(width_);
  }
  
  basic_ios<CharT, Traits>& stream_;
  typename basic_ios<CharT, Traits>::fmtflags const flags_;
  streamsize const width_;
  streamsize const precision_;
  CharT const fill_;
};

} // namespace rangeio_detail
} // namespace std

#endif  // STD_RANGEIO_stream_formatting_saver_
