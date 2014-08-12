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
 * specifically the input version that overwrites the range.
 * 
 * These tests are not meant to be exhaustive, merely illustrative.
 */

#include <array>
#include <forward_list>
#include <iterator>
#include <list>
#include <sstream>
#include <type_traits>
#include <vector>

#include <rangeio>

#include "gtest/gtest.h"

/* Test: Verify the types associated with overwrite() are correct.
 * 
 * The return value of overwrite() should be an object with a member function
 * named count() member function that returns size_t, and a member function
 * named next() that returns an iterator to the range.
 */
TEST(Overwrite, Types)
{
  auto v = std::vector<double>{};
  auto a = std::array<std::string, 4>{};
  
  EXPECT_TRUE((std::is_same<std::size_t, decltype(std::overwrite(v).count())>::value));
  EXPECT_TRUE((std::is_same<std::size_t, decltype(std::overwrite(a).count())>::value));
  
  EXPECT_TRUE((std::is_same<decltype(v.begin()), decltype(std::overwrite(v).next())>::value));
  EXPECT_TRUE((std::is_same<decltype(a.begin()), decltype(std::overwrite(a).next())>::value));
}

/* Test: Input into a range using overwrite().
 * 
 * Everything in the input sequence that can be converted to the range's value
 * type should be read until either the range is completely overwritten, or a
 * conversion error, an I/O error, or EOF.
 */
TEST(Overwrite, Input)
{
  {
    auto r = std::array<double, 4>{};
    
    std::istringstream iss{"12.3 .34 1e5 -.1"};
    iss.imbue(std::locale::classic());
    
    EXPECT_TRUE(iss >> std::overwrite(r));
    EXPECT_TRUE(iss.eof());
    EXPECT_FALSE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{4}, r.size());
    EXPECT_EQ(12.3, r.at(0));
    EXPECT_EQ(.34, r.at(1));
    EXPECT_EQ(1e5, r.at(2));
    EXPECT_EQ(-.1, r.at(3));
  }
  {
    auto r = std::forward_list<int>(4, 0);
    
    std::istringstream iss{"42 69 57 x"};
    iss.imbue(std::locale::classic());
    
    EXPECT_FALSE(iss >> std::overwrite(r));
    EXPECT_FALSE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    auto i = r.cbegin();
    EXPECT_EQ(4, std::distance(i, r.cend()));
    EXPECT_EQ(42, *i++);
    EXPECT_EQ(69, *i++);
    EXPECT_EQ(57, *i++);
    EXPECT_EQ(0, *i++);
    
    iss.clear();
    auto c = 'a';
    EXPECT_TRUE(iss >> c);
    EXPECT_EQ('x', c);
  }
}

/* Test: Error checking on range streaming using overwrite().
 * 
 * Everything in the input sequence that can be converted to the range's value
 * type should be read until either the range is completely overwritten, or a
 * conversion error, an I/O error, or EOF.
 * The number of elements read should be returned by the count() member
 * function of the returned object. The next() member should return an iterator
 * to the next location that will be overwritten.
 */
TEST(Overwrite, ErrorChecking)
{
  {
    auto r = std::array<int, 5>{};
    
    std::istringstream iss{"16 32 64 x"};
    iss.imbue(std::locale::classic());
    
    auto proxy_object = std::overwrite(r);
    EXPECT_EQ(std::size_t{0}, proxy_object.count());
    EXPECT_TRUE(r.begin() == proxy_object.next());
    
    EXPECT_FALSE(iss >> proxy_object);
    EXPECT_FALSE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{3}, proxy_object.count());
    EXPECT_TRUE((r.begin() + 3) == proxy_object.next());
    
    EXPECT_EQ(std::size_t{5}, r.size());
    EXPECT_EQ(16, r.at(0));
    EXPECT_EQ(32, r.at(1));
    EXPECT_EQ(64, r.at(2));
    EXPECT_EQ(0, r.at(3));
    EXPECT_EQ(0, r.at(4));
    
    iss.clear();
    auto c = 'a';
    EXPECT_TRUE(iss >> c);
    EXPECT_EQ('x', c);
  }
  {
    auto r = std::array<int, 2>{};
    
    std::istringstream iss{"16 32 64 x"};
    iss.imbue(std::locale::classic());
    
    auto proxy_object = std::overwrite(r);
    EXPECT_EQ(std::size_t{0}, proxy_object.count());
    EXPECT_TRUE(r.begin() == proxy_object.next());
    
    EXPECT_TRUE(iss >> proxy_object);
    EXPECT_FALSE(iss.eof());
    EXPECT_FALSE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{2}, proxy_object.count());
    EXPECT_TRUE(r.end() == proxy_object.next());
    
    EXPECT_EQ(std::size_t{2}, r.size());
    EXPECT_EQ(16, r.at(0));
    EXPECT_EQ(32, r.at(1));
  }
}

/* Test: Formatting when using overwrite().
 * 
 * Whatever the formatting state at the beginning of the input of a streamed
 * range, it should be applied to every element in the range.
 */
TEST(Overwrite, Formatting)
{
  {
    auto r = std::vector<std::string>(4);
    
    std::istringstream iss{"abcdefg"};
    iss.width(2);
    
    EXPECT_TRUE(iss >> overwrite(r));
    EXPECT_TRUE(iss.eof());
    EXPECT_FALSE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{4}, r.size());
    EXPECT_EQ("ab", r.at(0));
    EXPECT_EQ("cd", r.at(1));
    EXPECT_EQ("ef", r.at(2));
    EXPECT_EQ("g", r.at(3));
  }
  {
    auto r = std::array<int, 3>{};
    
    std::istringstream iss{"10 0x10 010"};
    iss.imbue(std::locale::classic());
    iss.setf(std::ios_base::hex, std::ios_base::basefield);
    
    EXPECT_TRUE(iss >> overwrite(r));
    EXPECT_TRUE(iss.eof());
    EXPECT_FALSE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{3}, r.size());
    EXPECT_EQ(0x10, r.at(0));
    EXPECT_EQ(0x10, r.at(1));
    EXPECT_EQ(0x10, r.at(2));
  }
}
