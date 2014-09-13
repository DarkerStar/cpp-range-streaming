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

#ifndef STD_RANGEIO_input_
#define STD_RANGEIO_input_

#include <iosfwd>
#include <iterator>
#include <limits>
#include <tuple>
#include <utility>

#include "stream-formatting-saver.hpp"

namespace std {
namespace rangeio_detail {

/** Helper template to deduce the iterator type of a range.
 * 
 * \tparam Range  The range to deduce the iterator type of.
 */
template <typename Range>
using iterator_type_of = decltype(begin(declval<Range&>()));

/** Helper template to deduce the value type of a range.
 * 
 * \tparam Range  The range to deduce the value type of.
 */
template <typename Range>
using value_type_of = typename iterator_traits<iterator_type_of<Range>>::value_type;

#ifdef DOXYGEN_RUNNING
/** The interface required for the input behaviour type.
 * 
 * The second argument to the input() function must be an instance
 * of a class that matches this class's interface. Changing the
 * implementations of the functions changes the behaviour of the
 * input operation.
 * 
 * \tparam Range     The range type being read into.
 * \tparam Iterator  An iterator type for the range.
 */
template <typename Range, typename Iterator>
struct basic_input_behaviour
{
  /** Prepares the input operation.
   * 
   * This function is called once at the beginning of every input
   * operation, before any input is attempted. The primary purpose
   * is to initialize the \c next iterator.
   * 
   * Other possible uses include initializing counters for reading
   * only a certain number of values, or reserving space in the
   * range.
   * 
   * \param  r   The range being read into.
   * \param  i   An iterator to the next location in the range to
   *             read into.
   * 
   * \return   A tuple containing:
   *             - \c true if input should continue,
   *               \c false if it should be aborted.
   *             - the value that \c next should be initialized to.
   */
  auto prepare(
      Range& r,
      Iterator i) ->
    tuple<bool, Iterator>;
  
  /** Reads a single value from the stream and stores it in the range.
   * 
   * This function is called repeatedly during the input operation,
   * once for every element read. It must read one element from
   * the stream \a in and store it in the range \a r in some way -
   * presumably at the location referenced by \a i - then report
   * the result via the return value.
   * 
   * This function is not guaranteed to be called during an input
   * operation - it will not be called if prepare() returns
   * \c false or if the stream is in a fail state.
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
   *             - \c true if input should continue,
   *               \c false if it should be aborted.
   *             - the value that \c next should be set to.
   *             - \c true if a value was successfully read,
   *               \c false if not.
   *             - \c true if the value read was stored in
   *               the range, \c false if it was not stored.
   */
  template <typename CharT, typename Traits>
  auto read(
      basic_istream<CharT, Traits>& in,
      Range& r,
      Iterator i) ->
    tuple<bool, Iterator, bool, bool>;
};
#endif  // DOXYGEN_RUNNING

/** Range input operation type.
 * 
 * This is the type returned by all the range input functions. It
 * handles keeping track of the \c next iterator, and the \c count
 * of elements read from the input stream in the last input
 * operation, and the number of elements actually \c stored in the
 * range in the last input operation.
 * 
 * \tparam Range     The range type being written to.
 * \tparam Iterator  The iterator type.
 * \tparam Behaviour The input operation behaviour type.
 */
template <typename Range, typename Iterator, typename Behaviour>
struct range_input_operation
{
  /** Constructs a range input operation object.
   * 
   * Initializes the \c count of read elements and the count of
   * \c stored elements to zero.
   * 
   * \param   r   The range that will be written to.
   * \param   i   The initial value of the \c next iterator.
   * \param   b   The input behaviour.
   */
  range_input_operation(Range& r, Iterator i, Behaviour b) :
    range_{r},
    op_(move(b)),
    next{i}
  {}
  
  //! Reference to the range being written to.
  Range& range_;
  
  //! The behaviour of the input operation.
  Behaviour op_;
  
  //! The number of values read during the last input operation.
  size_t count = 0;
  
  //! The number of values written to the range during the last input operation.
  size_t stored = 0;
  
  //! Iterator references the next position that will be read into.
  Iterator next;
};

/** Extraction operator for range input.
 * 
 * This function handles pretty much all of the logic for range
 * input. It uses the <tt>prepare()</tt> and <tt>read()</tt> members
 * of the \c Behaviour object, setting the counts to zero and
 * calling <tt>prepare()</tt> before attempting any input. If the
 * input can continue, the function begins calling <tt>read()</tt>
 * in a loop until input is complete (as determined by the
 * \c Behaviour object), handling incrementing \c count and
 * \c stored and the stream formatting.
 * 
 * \param   in  The stream to read from.
 * \param   p   The range input object.
 * 
 * \tparam  Range     The range type to read into.
 * \tparam  Iterator  The type of the iterator.
 * \tparam  Behaviour The input behaviour.
 * \tparam  CharT     The input stream character type.
 * \tparam  Traits    The input stream character traits type.
 * 
 * \return  \a in .
 */
template <typename Range, typename Iterator, typename Behaviour, typename CharT, typename Traits>
auto operator>>(basic_istream<CharT, Traits>& in, range_input_operation<Range, Iterator, Behaviour>& p) ->
  basic_istream<CharT, Traits>&
{
  p.count = 0;
  p.stored = 0;
  
  auto continue_input = false;
  tie(continue_input, p.next) = p.op_.prepare(p.range_, p.next);
  
  if (continue_input)
  {
    auto const formatting = stream_formatting_saver<CharT, Traits>{in};
    
    while (in && continue_input)
    {
      formatting.restore();
      
      auto value_read   = false;
      auto value_stored = false;
      
      tie(continue_input, p.next, value_read, value_stored) =
        p.op_.read(in, p.range_, p.next);
      
      if (value_read)
        ++p.count;
      
      if (value_stored)
        ++p.stored;
    }
  }
  
  in.width(0);
  
  return in;
}

/** Extraction operator for range input.
 * 
 * This version simply calls the lvalue reference version. It is
 * only necessary to handle the rvalue use case.
 * 
 * \param   in  The stream to read from.
 * \param   p   The range input object.
 * 
 * \tparam  Range     The range type to read into.
 * \tparam  Iterator  The type of the iterator.
 * \tparam  Behaviour The input behaviour.
 * \tparam  CharT     The input stream character type.
 * \tparam  Traits    The input stream character traits type.
 * 
 * \return  \a in .
 */
template <typename Range, typename Iterator, typename Behaviour, typename CharT, typename Traits>
auto operator>>(basic_istream<CharT, Traits>& in, range_input_operation<Range, Iterator, Behaviour>&& p) ->
  basic_istream<CharT, Traits>&
{
  return in >> p;
}

} // namespace rangeio_detail

/** Generic range input function.
 * 
 * Constructs a range input operation object with the supplied
 * behaviour. Providing your own behaviour type means you can
 * easily adapt range input to any behaviour you please.
 * 
 * \param   r   The range to write values to.
 * \param   i   An iterator into the range.
 * \param   b   The input behaviour.
 * 
 * \tparam  Range     The range type to read into.
 * \tparam  Iterator  The type of the iterator.
 * \tparam  Behaviour The input behaviour.
 * 
 * \return  A range input operation object for the given range,
 *          with the desired behaviour.
 */
template <typename Range, typename Iterator, typename Behaviour>
auto input(Range& r, Iterator i, Behaviour b) ->
  rangeio_detail::range_input_operation<Range, Iterator, Behaviour>
{
  return {r, i, move(b)};
}

/** Generic range input function.
 * 
 * Constructs a range input operation object with the supplied
 * behaviour. Providing your own behaviour type means you can
 * easily adapt range input to any behaviour you please.
 * 
 * \param   r   The range to write values to.
 * \param   b   The input behaviour.
 * 
 * \tparam  Range     The range type to read into.
 * \tparam  Behaviour The input behaviour.
 * 
 * \return  A range input operation object for the given range,
 *          with the desired behaviour.
 */
template <typename Range, typename Behaviour>
auto input(Range& r, Behaviour b) ->
  rangeio_detail::range_input_operation<Range, typename tuple_element<1, decltype(b.prepare())>::type, Behaviour>
{
  auto i = get<1>(b.prepare(r, begin(r)));
  
  return {r, i, move(b)};
}

} // namespace std

#include "overwrite.hpp"
#include "back_insert.hpp"
#include "front_insert.hpp"
#include "insert.hpp"

#endif // STD_RANGEIO_input_
