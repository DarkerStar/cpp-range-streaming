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

#ifndef STD_RANGEIO_back_insert_
#define STD_RANGEIO_back_insert_

#include "input.hpp"

namespace std {
namespace rangeio_detail {

/** Back inserting range input behaviour type.
 * 
 * \tparam Range     The range type being appended to.
 */
template <typename Range>
struct back_insert_behaviour
{
  /** Constructs a back insert behaviour object.
   * 
   * \param   n   The number of elements to read in a single read operation.
   */
  back_insert_behaviour(size_t n = numeric_limits<size_t>::max()) :
    v_{},
    n_{n},
    current_{0}
  {}
  
  /** Prepares the input operation.
   * 
   * Sets \c next to <tt>end(r)</tt>.
   * 
   * \param  r   The range being read into.
   * \param  i   Unused.
   * 
   * \return   A tuple containing:
   *             - \c true .
   *             - <tt>end(r)</tt>.
   */
  auto prepare(Range& r, iterator_type_of<Range>) ->
    tuple<bool, iterator_type_of<Range>>
  {
    current_ = 0;
    return make_tuple(true, end(r));
  }
  
  /** Reads a single value from the stream and appends it to the range.
   * 
   * Attempts to read a value from \a in and - if successful - uses
   * <tt>r.push_back()</tt> to move the value into the range.
   * 
   * \param  in  The stream being read.
   * \param  r   The range being read into.
   * \param  i   Unused.
   * 
   * \tparam CharT   The character type of the stream being read.
   * \tparam Traits  The character traits of the stream being read.
   * 
   * \return   A tuple containing:
   *             - \c true if input succeeded, \c false otherwise.
   *             - <tt>end(r)</tt>.
   *             - \c true if input succeeded, \c false otherwise.
   *             - \c true if input succeeded, \c false otherwise.
   */
  template <typename CharT, typename Traits>
  auto read(basic_istream<CharT, Traits>& in, Range& r, iterator_type_of<Range> i) ->
    tuple<bool, iterator_type_of<Range>, bool, bool>
  {
    if ((current_ < n_) && (in >> v_))
    {
      r.push_back(move(v_));
      return make_tuple(++current_ < n_, end(r), true, true);
    }
    
    return make_tuple(false, i, false, false);
  }
  
  //! An instance of the range's value type, to use as a buffer for reading
  //! into.
  value_type_of<Range> v_;
  
  //! The number of elements to read in a single read operation.
  size_t const n_ = numeric_limits<size_t>::max();
  
  //! The number of elements read so far in the current read operation.
  size_t current_ = 0;
};

} // namespace rangeio_detail

/** Back insert range input function.
 * 
 * \param   r   The range to write values to.
 * 
 * \tparam  Range     The range type to read into.
 * 
 * \return  A range input operation object for the given range, with the desired
 *          behaviour.
 */
template <typename Range>
auto back_insert(Range& r) ->
  rangeio_detail::range_input_operation<Range, rangeio_detail::iterator_type_of<Range>, rangeio_detail::back_insert_behaviour<Range>>
{
  return input(r, end(r), rangeio_detail::back_insert_behaviour<Range>{});
}

/** Back insert range input function.
 * 
 * \param   r   The range to write values to.
 * \param   n   The maximum number of values to read in a single input
 *              operation.
 * 
 * \tparam  Range     The range type to read into.
 * 
 * \return  A range input operation object for the given range, with the desired
 *          behaviour.
 */
template <typename Range>
auto back_insert_n(Range& r, size_t n) ->
  rangeio_detail::range_input_operation<Range, rangeio_detail::iterator_type_of<Range>, rangeio_detail::back_insert_behaviour<Range>>
{
  return input(r, end(r), rangeio_detail::back_insert_behaviour<Range>{n});
}

} // namespace std

#endif // STD_RANGEIO_back_insert_
