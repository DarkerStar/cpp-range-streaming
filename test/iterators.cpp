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
 * using iterators. Both input and output is tested.
 * 
 * These tests are not meant to be exhaustive, merely illustrative.
 */

#include <algorithm>
#include <array>
#include <iterator>
#include <locale>
#include <sstream>
#include <string>

#include <rangeio>

#include <boost/range/iterator_range.hpp>

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

/* Test: Input into a range defined by a pair of iterators.
 * 
 * The "basic" use of std::stream_iterator_range() with input streams, replaces
 * each element in the range by reading a new value from the input stream.
 * Should leave the stream in the same state it would be in if the values were
 * read by a manual loop.
 */
TEST(Iterators, Input)
{
  // Note: source data has 6 doubles, expected has 5. That means when the
  // source data is read, there should be one more double left. This will
  // be checked.
  auto const src = std::string{"1.1 2.2 3.3 4.4 5.5 6.6"};
  auto const expected = std::array<double, 5>{ 1.1, 2.2, 3.3, 4.4, 5.5 };
  
  auto a = std::array<double, 5>{};
  auto d = 0.0;
  
  std::istringstream iss{src};
  iss.imbue(std::locale::classic());
  
  auto r = boost::make_iterator_range(a.begin(), a.end());
  EXPECT_TRUE(iss >> std::overwrite(r));
  EXPECT_TRUE(std::equal(a.begin(), a.end(), expected.begin()));
  
  EXPECT_TRUE(iss >> d);
  EXPECT_EQ(6.6, d);
}

/* Test: Output of a range given by an iterator pair.
 * 
 * std::stream_iterator_range(b, e) is just
 * std::stream_range(make_range(b, e)), where make_range() is a function
 * that creates a simple object with begin() and end() functions that just
 * return b and e respectively.
 */
TEST(Iterators, Output)
{
  {
    auto const r = std::array<char, 4>{ 'a', 'b', 'c', 'd' };
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::write_all(boost::make_iterator_range(r.begin(), r.end())));
    EXPECT_EQ("abcd", oss.str());
  }
  
  {
    auto const src = std::string{"32 48 59 61"};
    
    std::istringstream in{src};
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::write_all(
      boost::make_iterator_range(
        std::istream_iterator<int>{in}, std::istream_iterator<int>{})));
    EXPECT_EQ("32485961", oss.str());
  }
}

/* Test: Output of a range given by an iterator pair with a delimiter.
 * 
 * std::stream_iterator_range(b, e, d) is just
 * std::stream_range(make_range(b, e), d), where make_range() is a function
 * that creates a simple object with begin() and end() functions that just
 * return b and e respectively.
 */
TEST(Iterators, DelimitedOutput)
{
  auto const r = std::array<char, 4>{ 'a', 'b', 'c', 'd' };
  
  // Lvalue delimiter.
  {
    noncopyable_nonmoveable_delimiter d{"***"};
    
    std::ostringstream oss;
    
    EXPECT_TRUE(oss << std::write_all(boost::make_iterator_range(r.begin(), r.end()), d));
    EXPECT_EQ("a***b***c***d", oss.str());
  }
  
  // Rvalue delimiter.
  {
    std::ostringstream oss;
    
    EXPECT_TRUE(oss << std::write_all(boost::make_iterator_range(r.begin(), r.end()), noncopyable_delimiter{"++"}));
    EXPECT_EQ("a++b++c++d", oss.str());
  }
  
  // "Special" delimiter.
  {
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::write_all(boost::make_iterator_range(r.begin(), r.end()), incrementing_integer_delimiter{}));
    EXPECT_EQ("a0b1c2d", oss.str());
  }
}
