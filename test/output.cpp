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

/* 
 * This file contains the tests for the proposed range streaming facilities,
 * using output. Both the versions with and without delimiters are tested
 * separately.
 * 
 * These tests are not meant to be exhaustive, merely illustrative.
 */

#include <algorithm>
#include <array>
#include <forward_list>
#include <iterator>
#include <locale>
#include <sstream>
#include <string>

#include <stream_range>

#include "gtest/gtest.h"

/* 
 * Some extra types used in the tests are contained in this namespace. Most of
 * them are basically identical to standard library classes like array or
 * string, except deliberately non-copyable and/or non-moveable. This is to
 * make sure things work even with types that are non-copyable, non-moveable,
 * and non-default-constructable.
 */
namespace {

/* 
 * A noncopyable version of std::array.
 */
template <typename T, std::size_t N>
struct noncopyable_range : std::array<T, N>
{
  noncopyable_range(std::initializer_list<T> i)
  {
    std::copy_n(i.begin(), std::min(N, i.size()), this->begin());
  }
  
  noncopyable_range(noncopyable_range const&) = delete;
  noncopyable_range(noncopyable_range&&) = default;
  
  auto operator=(noncopyable_range const&) -> noncopyable_range& = delete;
  auto operator=(noncopyable_range&&) -> noncopyable_range& = default;
};

/* 
 * A noncopyable and nonmoveable version of std::array.
 */
template <typename T, std::size_t N>
class noncopyable_nonmoveable_range :
  private std::array<T, N>
{
public:
  using std::array<T, N>::iterator;
  using std::array<T, N>::const_iterator;
  
  using std::array<T, N>::begin;
  using std::array<T, N>::end;
  
  noncopyable_nonmoveable_range(std::initializer_list<T> i)
  {
    std::copy_n(i.begin(), std::min(N, i.size()), this->begin());
  }
  
  noncopyable_nonmoveable_range(noncopyable_nonmoveable_range const&) = delete;
  noncopyable_nonmoveable_range(noncopyable_nonmoveable_range&&) = delete;
  
  auto operator=(noncopyable_nonmoveable_range const&) -> noncopyable_nonmoveable_range& = delete;
  auto operator=(noncopyable_nonmoveable_range&&) -> noncopyable_nonmoveable_range& = delete;
};

/* 
 * A noncopyable version of std::string.
 */
class noncopyable_delimiter
{
public:
  explicit noncopyable_delimiter(std::string const& s) : s_{std::move(s)} {}
  
  noncopyable_delimiter(noncopyable_delimiter const&) = delete;
  noncopyable_delimiter(noncopyable_delimiter&&) = default;
  
  auto operator=(noncopyable_delimiter const&) -> noncopyable_delimiter& = delete;
  auto operator=(noncopyable_delimiter&&) -> noncopyable_delimiter& = default;
  
private:
  std::string s_;
  
  friend auto operator<<(std::ostream&, noncopyable_delimiter const&) -> std::ostream&;
};

auto operator<<(std::ostream& out, noncopyable_delimiter const& d) -> std::ostream&
{
  return out << d.s_;
}

/* 
 * A noncopyable and nonmoveable version of std::string.
 */
class noncopyable_nonmoveable_delimiter
{
public:
  explicit noncopyable_nonmoveable_delimiter(std::string const& s) : s_{std::move(s)} {}
  
  noncopyable_nonmoveable_delimiter(noncopyable_nonmoveable_delimiter const&) = delete;
  noncopyable_nonmoveable_delimiter(noncopyable_nonmoveable_delimiter&&) = delete;
  
  auto operator=(noncopyable_nonmoveable_delimiter const&) -> noncopyable_nonmoveable_delimiter& = delete;
  auto operator=(noncopyable_nonmoveable_delimiter&&) -> noncopyable_nonmoveable_delimiter& = delete;
  
private:
  std::string s_;
  
  friend auto operator<<(std::ostream&, noncopyable_nonmoveable_delimiter const&) -> std::ostream&;
};

auto operator<<(std::ostream& out, noncopyable_nonmoveable_delimiter const& d) -> std::ostream&
{
  return out << d.s_;
}

/* 
 * A special type intended to be used as a delimiter. What makes this type
 * special is that every time it is streamed out, its value increments. This is
 * to demonstrate that delimiters need not be constant values.
 */
struct incrementing_integer_delimiter
{
  mutable unsigned i = 0u;
};

template <typename CharT, typename Traits>
auto operator<<(std::basic_ostream<CharT, Traits>& out, incrementing_integer_delimiter const& iid) ->
  std::basic_ostream<CharT, Traits>&
{
  out << iid.i;
  ++iid.i;
  
  return out;
}

} // anonymous namespace

/* Test: Output of a range when the range is given as an lvalue.
 * 
 * The "basic" use of std::stream_range() with output streams, inserts each
 * element in the range into the output stream.
 * 
 * When the range is given as an lvalue, it should be taken by reference. No
 * copies should be made.
 */
TEST(Output, LvalueRange)
{
  {
    // Try it with a forward list because that has the most restricted iterator
    // type (other than istream iterators).
    auto r = std::forward_list<int>{ 1, 1, 2, 3, 5, 8 };
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::stream_range(r));
    EXPECT_EQ(oss.str(), "112358");
  }
  
  {
    // Use this special type to make absolutely sure that no copies or moves
    // of the range is done (when the range is an lvalue).
    noncopyable_nonmoveable_range<char, 5> const r{ 'w', 'o', 'r', 'k', 's' };
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::stream_range(r));
    EXPECT_EQ(oss.str(), "works");
  }
}

/* Test: Output of a range when the range is given as an rvalue.
 * 
 * The "basic" use of std::stream_range() with output streams, inserts each
 * element in the range into the output stream.
 * 
 * When the range is given as an rvalue, it has to copy the range. Actually,
 * it has to *move* the range, and keep it until output is complete.
 */
TEST(Output, RvalueRange)
{
  {
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::stream_range(std::array<int, 4>{ 2, 4, 6, 8 }));
    EXPECT_EQ(oss.str(), "2468");
  }
  
  {
    // Use this special type to make absolutely sure that no copies of the
    // are made (when the range is an rvalue) - though it can be moved
    // (necessary because it has to be passed through to the object returned
    // by std::stream_range()).
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::stream_range(noncopyable_range<int, 3>{ -1, -2, -3 }));
    EXPECT_EQ(oss.str(), "-1-2-3");
  }
}

/* Test: Output of a range given by an iterator pair.
 * 
 * std::stream_iterator_range(b, e) is just
 * std::stream_range(make_range(b, e)), where make_range() is a function
 * that creates a simple object with begin() and end() functions that just
 * return b and e respectively.
 */
TEST(Output, Iterators)
{
  {
    auto const r = std::array<char, 4>{ 'a', 'b', 'c', 'd' };
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::stream_iterator_range(r.begin(), r.end()));
    EXPECT_EQ(oss.str(), "abcd");
  }
  
  {
    auto const src = std::string{"32 48 59 61"};
    
    std::istringstream in{src};
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::stream_iterator_range(
      std::istream_iterator<int>{in}, std::istream_iterator<int>{}));
    EXPECT_EQ(oss.str(), "32485961");
  }
}

/* Test: Preservation of formatting from element to element.
 * 
 * Whatever the formatting state at the beginning of the output of a streamed
 * range, it should be applied to every element in the range.
 */
TEST(Output, Formatting)
{
  {
    auto const r = std::array<int, 5>{ 0x0287, 0x071A, 0x00E6, 0x001A, 0x029E };
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss.width(8);
    oss.fill('.');
    oss.setf(std::ios_base::hex, std::ios_base::basefield);
    oss.setf(std::ios_base::left, std::ios_base::adjustfield);
    oss.setf(std::ios_base::uppercase);
    oss.setf(std::ios_base::showbase);
    
    EXPECT_TRUE(oss << std::stream_range(r));
    EXPECT_EQ(oss.str(), "0X287..." "0X71A..." "0XE6...." "0X1A...." "0X29E...");
  }
  
  {
    auto const r = std::array<double, 5>{ 1.0, -2.3, 6.66666, -0.12345, -1.2345 };
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss.width(8);
    oss.precision(3);
    oss.fill('_');
    oss.setf(std::ios_base::internal, std::ios_base::adjustfield);
    oss.setf(std::ios_base::showpoint);
    
    EXPECT_TRUE(oss << std::stream_range(r));
    EXPECT_EQ(oss.str(), "____1.00" "-___2.30" "____6.67" "-__0.123" "-___1.23");
  }
}

/* Test: Output of a range with a delimiter, when the range is given as an
 *       lvalue.
 * 
 * Using std::stream_range() with a delimiter, inserts each element in the range
 * into the output stream with the delimiter between.
 */
TEST(DelimitedOutput, LvalueRange)
{
  noncopyable_nonmoveable_range<char, 5> const r{ 'w', 'o', 'r', 'k', 's' };
  
  // Lvalue delimiter.
  {
    noncopyable_nonmoveable_delimiter d{"***"};
    
    std::ostringstream oss;
    
    EXPECT_TRUE(oss << std::stream_range(r, d));
    EXPECT_EQ(oss.str(), "w***o***r***k***s");
  }
  
  // Rvalue delimiter.
  {
    std::ostringstream oss;
    
    EXPECT_TRUE(oss << std::stream_range(r, noncopyable_delimiter{"++"}));
    EXPECT_EQ(oss.str(), "w++o++r++k++s");
  }
  
  // "Special" delimiter.
  {
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::stream_range(r, incrementing_integer_delimiter{}));
    EXPECT_EQ(oss.str(), "w0o1r2k3s");
  }
}

/* Test: Output of a range with a delimiter, when the range is given as an
 *       rvalue.
 * 
 * Using std::stream_range() with a delimiter, inserts each element in the range
 * into the output stream with the delimiter between.
 */
TEST(DelimitedOutput, RvalueRange)
{
  // Lvalue delimiter.
  {
    noncopyable_nonmoveable_delimiter d{"%*"};
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::stream_range(noncopyable_range<int, 4>{ 2, 4, 6, 8 }, d));
    EXPECT_EQ(oss.str(), "2%*4%*6%*8");
  }
  
  // Rvalue delimiter.
  {
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::stream_range(noncopyable_range<int, 4>{ 2, 4, 6, 8 }, noncopyable_delimiter{";"}));
    EXPECT_EQ(oss.str(), "2;4;6;8");
  }
  
  // "Special" delimiter.
  {
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::stream_range(noncopyable_range<int, 4>{ 2, 4, 6, 8 }, incrementing_integer_delimiter{}));
    EXPECT_EQ(oss.str(), "2041628");
  }
}

/* Test: Output of a range given by an iterator pair with a delimiter.
 * 
 * std::stream_iterator_range(b, e, d) is just
 * std::stream_range(make_range(b, e), d), where make_range() is a function
 * that creates a simple object with begin() and end() functions that just
 * return b and e respectively.
 */
TEST(DelimitedOutput, Iterators)
{
  auto const r = std::array<char, 4>{ 'a', 'b', 'c', 'd' };
  
  // Lvalue delimiter.
  {
    noncopyable_nonmoveable_delimiter d{"***"};
    
    std::ostringstream oss;
    
    EXPECT_TRUE(oss << std::stream_iterator_range(r.begin(), r.end(), d));
    EXPECT_EQ(oss.str(), "a***b***c***d");
  }
  
  // Rvalue delimiter.
  {
    std::ostringstream oss;
    
    EXPECT_TRUE(oss << std::stream_iterator_range(r.begin(), r.end(), noncopyable_delimiter{"++"}));
    EXPECT_EQ(oss.str(), "a++b++c++d");
  }
  
  // "Special" delimiter.
  {
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::stream_iterator_range(r.begin(), r.end(), incrementing_integer_delimiter{}));
    EXPECT_EQ(oss.str(), "a0b1c2d");
  }
}

/* Test: Preservation of formatting from element to element, with a delimiter.
 * 
 * Whatever the formatting state at the beginning of the output of a streamed
 * range, it should be applied to every element in the range. But the whatever
 * formatting state is after outputting an element, *that* state should be used
 * for the delimiter. So if s1 is the original state of the formatting flags
 * before any output, and s2 is the state after inserting an element of the
 * range's value_type, then the output should look like:
 *   <s1> element 1 <s2> delimiter <s1> element 2 <s2> delimiter <s1> element 3
 */
TEST(DelimitedOutput, Formatting)
{
  {
    auto const r = std::array<int, 5>{ 0x0287, 0x071A, 0x00E6, 0x001A, 0x029E };
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss.width(8);
    oss.fill('.');
    oss.setf(std::ios_base::hex, std::ios_base::basefield);
    oss.setf(std::ios_base::left, std::ios_base::adjustfield);
    oss.setf(std::ios_base::uppercase);
    oss.setf(std::ios_base::showbase);
    
    EXPECT_TRUE(oss << std::stream_range(r, 23));
    EXPECT_EQ(oss.str(), "0X287...0X17" "0X71A...0X17" "0XE6....0X17" "0X1A....0X17" "0X29E...");
  }
  
  {
    auto const r = std::array<double, 5>{ 1.0, -2.3, 6.66666, -0.12345, -1.2345 };
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss.width(8);
    oss.precision(3);
    oss.fill('_');
    oss.setf(std::ios_base::internal, std::ios_base::adjustfield);
    oss.setf(std::ios_base::showpoint);
    
    EXPECT_TRUE(oss << std::stream_range(r, -1.23456));
    EXPECT_EQ(oss.str(), "____1.00-1.23" "-___2.30-1.23" "____6.67-1.23" "-__0.123-1.23" "-___1.23");
  }
}
