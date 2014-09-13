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

#ifndef STD_RANGEIO_insert_
#define STD_RANGEIO_insert_

#include "input.hpp"

namespace std {
namespace rangeio_detail {

/** General inserting range input behaviour type.
 * 
 * \tparam Range     The range type being prepended to.
 */
template <typename Range, typename Iterator>
struct insert_behaviour
{
  /** Constructs an insert behaviour object.
   * 
   * \param   n   The number of elements to read in a single read
   *              operation.
   */
  insert_behaviour(size_t n = numeric_limits<size_t>::max()) :
    v_{},
    n_{n},
    current_{0}
  {}
  
  /** Prepares the input operation.
   * 
   * \param  r   The range being read into.
   * \param  i   An iterator to the next location in the range to
   *             read into.
   * 
   * \return   A tuple containing:
   *             - \c true .
   *             - \a i .
   */
  auto prepare(Range&, Iterator i) ->
    tuple<bool, Iterator>
  {
    current_ = 0;
    return make_tuple(true, i);
  }
  
  /** Reads a single value from the stream and inserts it into the range.
   * 
   * Attempts to read a value from \a in and - if successful - uses
   * <tt>r.insert()</tt> to move the value into the range at the
   * position referenced by \a i .
   * 
   * \param  in  The stream being read.
   * \param  r   The range being read into.
   * \param  i   An iterator to the next location in the range to
   *             read into.
   * 
   * \tparam CharT   The character type of the stream being read.
   * \tparam Traits  The character traits of the stream being read.
   * 
   * \return   A tuple containing:
   *             - \c true if input succeeded, \c false otherwise.
   *             - an iterator to the next position after the
   *               insert position, if input was successful, or
   *               \a i otherwise.
   *             - \c true if input succeeded, \c false otherwise.
   *             - \c true if input succeeded, \c false otherwise.
   */
  template <typename CharT, typename Traits>
  auto read(basic_istream<CharT, Traits>& in, Range& r, Iterator i) ->
    tuple<bool, Iterator, bool, bool>
  {
    if ((current_ < n_) && (in >> v_))
      return make_tuple(++current_ < n_, ++Iterator(r.insert(i, std::move(v_))), true, true);
    
    return make_tuple(false, i, false, false);
  }
  
  //! An instance of the range's value type, to use as a buffer
  //! for reading into.
  value_type_of<Range> v_;
  
  //! The number of elements to read in a single read operation.
  size_t const n_ = numeric_limits<size_t>::max();
  
  //! The number of elements read so far in the current read operation.
  size_t current_ = 0;
};

} // namespace rangeio_detail

/** General insert range input function.
 * 
 * \param   r   The range to write values to.
 * \param   i   The iterator referencing the position in the range
 *              to begin inserting values to.
 * 
 * \tparam  Range     The range type to read into.
 * 
 * \return  A range input operation object for the given range,
 *          with the desired behaviour.
 */
template <typename Range, typename Iterator>
auto insert(Range& r, Iterator i) ->
  rangeio_detail::range_input_operation<Range, Iterator, rangeio_detail::insert_behaviour<Range, Iterator>>
{
  return input(r, i, rangeio_detail::insert_behaviour<Range, Iterator>{});
}

/** General insert range input function.
 * 
 * \param   r   The range to write values to.
 * \param   i   The iterator referencing the position in the range
 *              to begin inserting values to.
 * \param   n   The maximum number of values to read in a single
 *              input operation.
 * 
 * \tparam  Range     The range type to read into.
 * 
 * \return  A range input operation object for the given range,
 *          with the desired behaviour.
 */
template <typename Range, typename Iterator>
auto insert_n(Range& r, Iterator i, size_t n) ->
  rangeio_detail::range_input_operation<Range, Iterator, rangeio_detail::insert_behaviour<Range, Iterator>>
{
  return input(r, i, rangeio_detail::insert_behaviour<Range, Iterator>{n});
}

} // namespace std

#endif // STD_RANGEIO_insert_
