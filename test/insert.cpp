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
 * specifically the input version that uses the range's insert() member
 * function to dynamically grow the range.
 * 
 * These tests are not meant to be exhaustive, merely illustrative.
 */

#include <iterator>
#include <list>
#include <sstream>
#include <type_traits>
#include <vector>

#include <rangeio>

#include "gtest/gtest.h"

/* Test: Verify the types associated with insert() are correct.
 * 
 * The return value of insert() should be an object with a member function
 * named count() member function that returns size_t, and a member function
 * named next() that returns an iterator to the range.
 */
TEST(Insert, Types)
{
  auto v = std::vector<double>{};
  auto l = std::list<std::string>{};
  
  EXPECT_TRUE((std::is_same<std::size_t, decltype(std::insert(v, v.begin()).count())>::value));
  EXPECT_TRUE((std::is_same<std::size_t, decltype(std::insert(l, l.begin()).count())>::value));
  EXPECT_TRUE((std::is_same<std::size_t, decltype(std::insert(v, v.begin(), v.front()).count())>::value));
  EXPECT_TRUE((std::is_same<std::size_t, decltype(std::insert(l, l.begin(), l.front()).count())>::value));
  
  EXPECT_TRUE((std::is_same<decltype(v.begin()), decltype(std::insert(v, v.begin()).next())>::value));
  EXPECT_TRUE((std::is_same<decltype(l.begin()), decltype(std::insert(l, l.begin()).next())>::value));
  EXPECT_TRUE((std::is_same<decltype(v.begin()), decltype(std::insert(v, v.begin(), v.front()).next())>::value));
  EXPECT_TRUE((std::is_same<decltype(l.begin()), decltype(std::insert(l, l.begin(), l.front()).next())>::value));
}

/* Test: Input into a range using insert().
 * 
 * Everything in the input sequence that can be converted to the range's value
 * type should be read until either a conversion error, an I/O error, or EOF.
 * All elements read should be appended to the back of the input sequence, in
 * order of reading.
 */
TEST(Insert, Input)
{
  {
    auto r = std::vector<double>{};
    
    std::istringstream iss{"12.3 .34 1e5 -.1"};
    iss.imbue(std::locale::classic());
    
    EXPECT_FALSE(iss >> std::insert(r, r.begin()));
    EXPECT_TRUE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{4}, r.size());
    EXPECT_EQ(12.3, r.at(0));
    EXPECT_EQ(.34, r.at(1));
    EXPECT_EQ(1e5, r.at(2));
    EXPECT_EQ(-.1, r.at(3));
  }
  {
    auto r = std::list<int>{ 0, 1, 5 };
    
    std::istringstream iss{"2 3 4 x"};
    iss.imbue(std::locale::classic());
    
    EXPECT_FALSE(iss >> std::insert(r, std::next(r.begin(), 2)));
    EXPECT_FALSE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{6}, r.size());
    for (auto i = r.cbegin(); i != r.cend(); ++i)
      EXPECT_EQ(std::distance(r.cbegin(), i), *i);
    
    iss.clear();
    auto c = 'a';
    EXPECT_TRUE(iss >> c);
    EXPECT_EQ('x', c);
  }
}

/* Test: Error checking on range streaming using insert().
 * 
 * Everything in the input sequence that can be converted to the range's value
 * type should be read until either a conversion error, an I/O error, or EOF.
 * The number of elements read should be returned by the count() member
 * function of the returned object. The next() member should return an iterator
 * to the next location that the next item read will be inserted before.
 */
TEST(Insert, ErrorChecking)
{
  {
    auto r = std::vector<int>{ 0, 4, 5 };
    
    std::istringstream iss{"1 2 3 x"};
    iss.imbue(std::locale::classic());
    
    auto proxy_object = std::insert(r, std::next(r.begin(), 1));
    EXPECT_EQ(std::size_t{0}, proxy_object.count());
    EXPECT_TRUE(std::next(r.begin(), 1) == proxy_object.next());
    
    EXPECT_FALSE(iss >> proxy_object);
    EXPECT_FALSE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{3}, proxy_object.count());
    EXPECT_TRUE(std::next(r.begin(), 4) == proxy_object.next());
    
    EXPECT_EQ(std::size_t{6}, r.size());
    for (auto i = r.cbegin(); i != r.cend(); ++i)
      EXPECT_EQ(std::distance(r.cbegin(), i), *i);
    
    iss.clear();
    auto c = 'a';
    EXPECT_TRUE(iss >> c);
    EXPECT_EQ('x', c);
  }
}

/* Test: Formatting when using insert().
 * 
 * Whatever the formatting state at the beginning of the input of a streamed
 * range, it should be applied to every element in the range.
 */
TEST(Insert, Formatting)
{
  {
    auto r = std::vector<std::string>{};
    
    std::istringstream iss{"abcdefg"};
    iss.width(2);
    
    EXPECT_FALSE(iss >> insert(r, r.end()));
    EXPECT_TRUE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{4}, r.size());
    EXPECT_EQ("ab", r.at(0));
    EXPECT_EQ("cd", r.at(1));
    EXPECT_EQ("ef", r.at(2));
    EXPECT_EQ("g", r.at(3));
  }
  {
    auto r = std::vector<int>{};
    
    std::istringstream iss{"10 0x10 010"};
    iss.imbue(std::locale::classic());
    iss.setf(std::ios_base::hex, std::ios_base::basefield);
    
    EXPECT_FALSE(iss >> insert(r, r.begin()));
    EXPECT_TRUE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{3}, r.size());
    EXPECT_EQ(0x10, r.at(0));
    EXPECT_EQ(0x10, r.at(1));
    EXPECT_EQ(0x10, r.at(2));
  }
}

/* Test: Verify the types associated with insert_n() are correct.
 * 
 * The return value of insert_n() should be an object with a member function
 * named count() member function that returns size_t, and a member function
 * named next() that returns an iterator to the range.
 */
TEST(InsertN, Types)
{
  auto v = std::vector<double>{};
  auto l = std::list<std::string>{};
  
  EXPECT_TRUE((std::is_same<std::size_t, decltype(std::insert_n(v, v.begin(), std::size_t{}).count())>::value));
  EXPECT_TRUE((std::is_same<std::size_t, decltype(std::insert_n(l, l.begin(), std::size_t{}).count())>::value));
  EXPECT_TRUE((std::is_same<std::size_t, decltype(std::insert_n(v, v.begin(), std::size_t{}, v.front()).count())>::value));
  EXPECT_TRUE((std::is_same<std::size_t, decltype(std::insert_n(l, l.begin(), std::size_t{}, l.front()).count())>::value));
  
  EXPECT_TRUE((std::is_same<decltype(v.begin()), decltype(std::insert_n(v, v.begin(), std::size_t{}).next())>::value));
  EXPECT_TRUE((std::is_same<decltype(l.begin()), decltype(std::insert_n(l, l.begin(), std::size_t{}).next())>::value));
  EXPECT_TRUE((std::is_same<decltype(v.begin()), decltype(std::insert_n(v, v.begin(), std::size_t{}, v.front()).next())>::value));
  EXPECT_TRUE((std::is_same<decltype(l.begin()), decltype(std::insert_n(l, l.begin(), std::size_t{}, l.front()).next())>::value));
}

/* Test: Input into a range using insert_n().
 * 
 * Everything in the input sequence that can be converted to the range's value
 * type should be read until either n items are read, or there is a
 * conversion error, an I/O error, or EOF. All elements read should be appended
 * to the back of the input sequence, in order of reading.
 */
TEST(InsertN, Input)
{
  {
    auto r = std::vector<double>{};
    
    std::istringstream iss{"12.3 .34 1e5 -.1"};
    iss.imbue(std::locale::classic());
    
    EXPECT_TRUE(iss >> std::insert_n(r, r.end(), 2));
    EXPECT_FALSE(iss.eof());
    EXPECT_FALSE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{2}, r.size());
    EXPECT_EQ(12.3, r.at(0));
    EXPECT_EQ(.34, r.at(1));
  }
  {
    auto r = std::vector<double>{};
    
    std::istringstream iss{"12.3 .34 1e5 -.1"};
    iss.imbue(std::locale::classic());
    
    EXPECT_FALSE(iss >> std::insert_n(r, r.begin(), 10));
    EXPECT_TRUE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{4}, r.size());
    EXPECT_EQ(12.3, r.at(0));
    EXPECT_EQ(.34, r.at(1));
    EXPECT_EQ(1e5, r.at(2));
    EXPECT_EQ(-.1, r.at(3));
  }
}
