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
 * This file contains the tests for the proposed range streaming facilities -
 * specifically the output version with delimiters.
 * 
 * These tests are not meant to be exhaustive, merely illustrative.
 */

#include <algorithm>
#include <array>
#include <forward_list>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <string>
#include <type_traits>

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

/* Test: Verify the types associated with the delimited write_all() are
 * correct.
 * 
 * The return value of write_all() is part of the specification - it must
 * return a properly-templated range_delimited_writer<>. And *that* type should
 * have a count() member function that returns size_t.
 */
TEST(WriteAllDelim, Types)
{
  auto v = std::vector<double>{};
  auto const a = std::array<std::string, 4>{};
  
  auto c = char{};
  auto const s = std::string{};
  
  EXPECT_TRUE((std::is_same<decltype(std::write_all(v, c)), std::range_delimited_writer<decltype(v)&, decltype(c)&>>::value));
  EXPECT_TRUE((std::is_same<decltype(std::write_all(a, c)), std::range_delimited_writer<decltype(a)&, decltype(c)&>>::value));
  EXPECT_TRUE((std::is_same<decltype(std::write_all(v, s)), std::range_delimited_writer<decltype(v)&, decltype(s)&>>::value));
  EXPECT_TRUE((std::is_same<decltype(std::write_all(a, s)), std::range_delimited_writer<decltype(a)&, decltype(s)&>>::value));
  
  EXPECT_TRUE((std::is_same<decltype(std::write_all(v, char{})), std::range_delimited_writer<decltype(v)&, char&&>>::value));
  EXPECT_TRUE((std::is_same<decltype(std::write_all(a, char{})), std::range_delimited_writer<decltype(a)&, char&&>>::value));
  EXPECT_TRUE((std::is_same<decltype(std::write_all(v, std::string{})), std::range_delimited_writer<decltype(v)&, std::string&&>>::value));
  EXPECT_TRUE((std::is_same<decltype(std::write_all(a, std::string{})), std::range_delimited_writer<decltype(a)&, std::string&&>>::value));
  
  EXPECT_TRUE((std::is_same<decltype(std::write_all(std::vector<double>{}, c)), std::range_delimited_writer<std::vector<double>&&, decltype(c)&>>::value));
  EXPECT_TRUE((std::is_same<decltype(std::write_all(std::array<std::string, 4>{}, c)), std::range_delimited_writer<std::array<std::string, 4>&&, decltype(c)&>>::value));
  EXPECT_TRUE((std::is_same<decltype(std::write_all(std::vector<double>{}, s)), std::range_delimited_writer<std::vector<double>&&, decltype(s)&>>::value));
  EXPECT_TRUE((std::is_same<decltype(std::write_all(std::array<std::string, 4>{}, s)), std::range_delimited_writer<std::array<std::string, 4>&&, decltype(s)&>>::value));
  
  EXPECT_TRUE((std::is_same<decltype(std::write_all(std::vector<double>{}, char{})), std::range_delimited_writer<std::vector<double>&&, char&&>>::value));
  EXPECT_TRUE((std::is_same<decltype(std::write_all(std::array<std::string, 4>{}, char{})), std::range_delimited_writer<std::array<std::string, 4>&&, char&&>>::value));
  EXPECT_TRUE((std::is_same<decltype(std::write_all(std::vector<double>{}, std::string{})), std::range_delimited_writer<std::vector<double>&&, std::string&&>>::value));
  EXPECT_TRUE((std::is_same<decltype(std::write_all(std::array<std::string, 4>{}, std::string{})), std::range_delimited_writer<std::array<std::string, 4>&&, std::string&&>>::value));
  
  EXPECT_TRUE((std::is_same<decltype(std::write_all(v, c).count()), std::size_t>::value));
}

/* Test: Output an lvalue range using write_all().
 * 
 * The entire range should be written to the output stream, with delimiters
 * in between each element.
 */
TEST(WriteAllDelim, LvalueRange)
{
  {
    auto r = std::forward_list<int>{42, 69, 57};
    
    std::ostringstream out{};
    out.imbue(std::locale::classic());
    
    auto c = ' ';
    EXPECT_TRUE(out << std::write_all(r, c));
    
    EXPECT_EQ("42 69 57", out.str());
  }
  {
    noncopyable_nonmoveable_range<double, 4> const r{12.3, .34, 1e-20, -.1};
    
    std::ostringstream out{};
    out.imbue(std::locale::classic());
    
    EXPECT_TRUE(out << std::write_all(r, ", "));
    
    EXPECT_EQ("12.3, 0.34, 1e-20, -0.1", out.str());
  }
  {
    auto const r = std::array<char, 5>{ 'w', 'o', 'r', 'k', 's' };
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::write_all(r, incrementing_integer_delimiter{}));
    EXPECT_EQ("w0o1r2k3s", oss.str());
  }
}

/* Test: Output an rvalue range using write_all().
 * 
 * The entire range should be written to the output stream, with delimiters
 * in between each element.
 */
TEST(WriteAllDelim, RvalueRange)
{
  {
    std::ostringstream out{};
    out.imbue(std::locale::classic());
    
    auto d = 0;
    EXPECT_TRUE(out << std::write_all(std::forward_list<int>{42, 69, 57}, d));
    
    EXPECT_EQ("42069057", out.str());
  }
  {
    std::ostringstream out{};
    out.imbue(std::locale::classic());
    
    EXPECT_TRUE(out << std::write_all(noncopyable_range<double, 4>{12.3, .34, 1e-20, -.1}, 'x'));
    
    EXPECT_EQ("12.3x0.34x1e-20x-0.1", out.str());
  }
  {
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    
    EXPECT_TRUE(oss << std::write_all(std::array<char, 5>{ 'w', 'o', 'r', 'k', 's' }, incrementing_integer_delimiter{}));
    EXPECT_EQ("w0o1r2k3s", oss.str());
  }
}

/* Test: Error checking on range streaming using write_all().
 * 
 * The entire range should be written to the output stream.
 * The number of elements read should be returned by the count() member
 * function of range_delimited_writer<Range>.
 */
TEST(WriteAllDelim, ErrorChecking)
{
  {
    noncopyable_nonmoveable_range<double, 4> const r{12.3, .34, 1e-20, -.1};
    
    std::ostringstream out{};
    out.imbue(std::locale::classic());
    
    auto d = std::string{", "};
    auto p = std::write_all(r, d);
    
    EXPECT_TRUE(out << p);
    
    EXPECT_EQ("12.3, 0.34, 1e-20, -0.1", out.str());
    EXPECT_EQ(std::size_t{4}, p.count());
  }
  {
    std::ostringstream out{};
    out.imbue(std::locale::classic());
    
    auto p = std::write_all(noncopyable_range<int, 3>{5, 6, 7}, '#');
    
    EXPECT_TRUE(out << p);
    
    EXPECT_EQ("5#6#7", out.str());
    EXPECT_EQ(std::size_t{3}, p.count());
  }
}

/* Test: Formatting when using write_all().
 * 
 * Whatever the formatting state at the beginning of the output of a streamed
 * range, it should be applied to every element in the range.
 * An empty range should just print the fill character width times.
 */
TEST(WriteAllDelim, Formatting)
{
  {
    auto const r = std::forward_list<int>{};
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss.width(4);
    oss.fill('*');
    
    EXPECT_TRUE(oss << std::write_all(r, '-'));
    EXPECT_EQ(0, oss.width());
    EXPECT_EQ("****", oss.str());
  }
  {
    auto const r = std::array<int, 5>{ 0x0287, 0x071A, 0x00E6, 0x001A, 0x029E };
    
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss.width(7);
    oss.fill('.');
    oss.setf(std::ios_base::hex, std::ios_base::basefield);
    oss.setf(std::ios_base::left, std::ios_base::adjustfield);
    oss.setf(std::ios_base::uppercase);
    oss.setf(std::ios_base::showbase);
    
    EXPECT_TRUE(oss << std::write_all(r, ' '));
    EXPECT_EQ("0X287.. 0X71A.. 0XE6... 0X1A... 0X29E..", oss.str());
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
    
    EXPECT_TRUE(oss << std::write_all(r, " | "));
    EXPECT_EQ("____1.00 | -___2.30 | ____6.67 | -__0.123 | -___1.23", oss.str());
  }
}
