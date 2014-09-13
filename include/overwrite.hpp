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

#ifndef STD_RANGEIO_overwrite_
#define STD_RANGEIO_overwrite_

#include "input.hpp"

namespace std {
namespace rangeio_detail {

/** Overwriting range input behaviour type.
 * 
 * \tparam Range     The range type being overwritten.
 */
template <typename Range>
struct overwrite_behaviour
{
  /** Prepares the input operation.
   * 
   * Simply returns \c true if the range is not empty, and gives \c begin(r) as
   * the value for \c next .
   * 
   * \param  r   The range being read into.
   * \param  i   Unused.
   * 
   * \return   A tuple containing:
   *             - \c true if the range is not empty (if it is empty, obviously
   *               nothing can be overwritten), \c false if it is empty.
   *             - <tt>begin(r)</tt>.
   */
  auto prepare(Range& r, iterator_type_of<Range>) ->
    tuple<bool, iterator_type_of<Range>>
  {
    return make_tuple(begin(r) != end(r), begin(r));
  }
  
  /** Reads a single value from the stream and stores it in the range.
   * 
   * If \a i is not equal to <tt>end(r)</tt>, reads an formats a value from
   * \a in , storing it in <tt>*i</tt>, then increments \a i .
   * 
   * \param  in  The stream being read.
   * \param  r   The range being read into.
   * \param  i   An iterator to the next location in the range to read into.
   * 
   * \tparam CharT   The character type of the stream being read.
   * \tparam Traits  The character traits of the stream being read.
   * 
   * \return   A tuple containing:
   *             - \c true if <tt>i != end(r)</tt> and input should continue,
   *               \c false otherwise.
   *             - the value that \c next should be set to.
   *             - \c true if a value was successfully read, \c false if not.
   *             - \c true if the value read was stored in the range, \c false
   *               if it was not stored.
   */
  template <typename CharT, typename Traits>
  auto read(basic_istream<CharT, Traits>& in, Range& r, iterator_type_of<Range> i) ->
    tuple<bool, iterator_type_of<Range>, bool, bool>
  {
    if (i != end(r) && (in >> *i))
    {
      ++i;
      return make_tuple(bool(i != end(r)), i, true, true);
    }
    
    return make_tuple(false, i, false, false);
  }
};

} // namespace rangeio_detail

/** Overwrite range input function.
 * 
 * \param   r   The range to write values to.
 * 
 * \tparam  Range     The range type to read into.
 * 
 * \return  A range input operation object for the given range, with the desired
 *          behaviour.
 */
template <typename Range>
auto overwrite(Range& r) ->
  rangeio_detail::range_input_operation<Range, rangeio_detail::iterator_type_of<Range>, rangeio_detail::overwrite_behaviour<Range>>
{
  return input(r, begin(r), rangeio_detail::overwrite_behaviour<Range>{});
}

} // namespace std

#endif // STD_RANGEIO_overwrite_
