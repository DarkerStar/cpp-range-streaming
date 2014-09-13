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
 * specifically the input version that uses the range's push_front() member
 * function to dynamically grow the range.
 * 
 * These tests are not meant to be exhaustive, merely illustrative.
 */

#include <deque>
#include <iterator>
#include <list>
#include <sstream>
#include <type_traits>

#include <rangeio>

#include "gtest/gtest.h"

/* Test: Verify the types associated with front_insert() are correct.
 * 
 * The return value of front_insert() should be an object with a member function
 * named count member function that returns size_t, and a member function
 * named next that returns an iterator to the range.
 */
TEST(FrontInsert, Types)
{
  auto v = std::deque<double>{};
  auto l = std::list<std::string>{};
  
  EXPECT_TRUE((std::is_same<std::size_t, decltype(std::front_insert(v).count)>::value));
  EXPECT_TRUE((std::is_same<std::size_t, decltype(std::front_insert(l).count)>::value));
  //EXPECT_TRUE((std::is_same<std::size_t, decltype(std::front_insert(v, v.front()).count)>::value));
  //EXPECT_TRUE((std::is_same<std::size_t, decltype(std::front_insert(l, l.front()).count)>::value));
  
  EXPECT_TRUE((std::is_same<decltype(v.begin()), decltype(std::front_insert(v).next)>::value));
  EXPECT_TRUE((std::is_same<decltype(l.begin()), decltype(std::front_insert(l).next)>::value));
  //EXPECT_TRUE((std::is_same<decltype(v.begin()), decltype(std::front_insert(v, v.front()).next)>::value));
  //EXPECT_TRUE((std::is_same<decltype(l.begin()), decltype(std::front_insert(l, l.front()).next)>::value));
}

/* Test: Input into a range using front_insert().
 * 
 * Everything in the input sequence that can be converted to the range's value
 * type should be read until either a conversion error, an I/O error, or EOF.
 * All elements read should be appended to the front of the input sequence, in
 * order of reading.
 */
TEST(FrontInsert, Input)
{
  {
    auto r = std::deque<double>{};
    
    std::istringstream iss{"12.3 .34 1e5 -.1"};
    iss.imbue(std::locale::classic());
    
    EXPECT_FALSE(iss >> std::front_insert(r));
    EXPECT_TRUE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{4}, r.size());
    EXPECT_EQ(-.1, r.at(0));
    EXPECT_EQ(1e5, r.at(1));
    EXPECT_EQ(.34, r.at(2));
    EXPECT_EQ(12.3, r.at(3));
  }
  {
    auto r = std::list<int>{ 3, 4, 5 };
    
    std::istringstream iss{"2 1 0 x"};
    iss.imbue(std::locale::classic());
    
    EXPECT_FALSE(iss >> std::front_insert(r));
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

/* Test: Error checking on range streaming using front_insert().
 * 
 * Everything in the input sequence that can be converted to the range's value
 * type should be read until either a conversion error, an I/O error, or EOF.
 * The number of elements read should be returned by the count member
 * function of  the returned object. The next member should return an iterator
 * to the beginning of the range.
 */
TEST(FrontInsert, ErrorChecking)
{
  {
    auto r = std::deque<int>{ 3, 4, 5 };
    
    std::istringstream iss{"2 1 0 x"};
    iss.imbue(std::locale::classic());
    
    auto proxy_object = std::front_insert(r);
    EXPECT_EQ(std::size_t{0}, proxy_object.count);
    EXPECT_TRUE(r.begin() == proxy_object.next);
    
    EXPECT_FALSE(iss >> proxy_object);
    EXPECT_FALSE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{3}, proxy_object.count);
    EXPECT_TRUE(r.begin() == proxy_object.next);
    
    EXPECT_EQ(std::size_t{6}, r.size());
    for (auto i = r.cbegin(); i != r.cend(); ++i)
      EXPECT_EQ(std::distance(r.cbegin(), i), *i);
    
    iss.clear();
    auto c = 'a';
    EXPECT_TRUE(iss >> c);
    EXPECT_EQ('x', c);
  }
}

/* Test: Formatting when using front_insert().
 * 
 * Whatever the formatting state at the beginning of the input of a streamed
 * range, it should be applied to every element in the range.
 */
TEST(FrontInsert, Formatting)
{
  {
    auto r = std::deque<std::string>{};
    
    std::istringstream iss{"abcdefg"};
    iss.width(2);
    
    EXPECT_FALSE(iss >> front_insert(r));
    EXPECT_TRUE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{4}, r.size());
    EXPECT_EQ("ab", r.at(3));
    EXPECT_EQ("cd", r.at(2));
    EXPECT_EQ("ef", r.at(1));
    EXPECT_EQ("g", r.at(0));
  }
  {
    auto r = std::deque<int>{};
    
    std::istringstream iss{"10 0x10 010"};
    iss.imbue(std::locale::classic());
    iss.setf(std::ios_base::hex, std::ios_base::basefield);
    
    EXPECT_FALSE(iss >> front_insert(r));
    EXPECT_TRUE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{3}, r.size());
    EXPECT_EQ(0x10, r.at(0));
    EXPECT_EQ(0x10, r.at(1));
    EXPECT_EQ(0x10, r.at(2));
  }
}

/* Test: Verify the types associated with front_insert_n() are correct.
 * 
 * The return value of front_insert_n() should be an object with a member function
 * named count member function that returns size_t, and a member function
 * named next that returns an iterator to the range.
 */
TEST(FrontInsertN, Types)
{
  auto v = std::deque<double>{};
  auto l = std::list<std::string>{};
  
  EXPECT_TRUE((std::is_same<std::size_t, decltype(std::front_insert_n(v, std::size_t{}).count)>::value));
  EXPECT_TRUE((std::is_same<std::size_t, decltype(std::front_insert_n(l, std::size_t{}).count)>::value));
  //EXPECT_TRUE((std::is_same<std::size_t, decltype(std::front_insert_n(v, std::size_t{}, v.front()).count)>::value));
  //EXPECT_TRUE((std::is_same<std::size_t, decltype(std::front_insert_n(l, std::size_t{}, l.front()).count)>::value));
  
  EXPECT_TRUE((std::is_same<decltype(v.begin()), decltype(std::front_insert_n(v, std::size_t{}).next)>::value));
  EXPECT_TRUE((std::is_same<decltype(l.begin()), decltype(std::front_insert_n(l, std::size_t{}).next)>::value));
  //EXPECT_TRUE((std::is_same<decltype(v.begin()), decltype(std::front_insert_n(v, std::size_t{}, v.front()).next)>::value));
  //EXPECT_TRUE((std::is_same<decltype(l.begin()), decltype(std::front_insert_n(l, std::size_t{}, l.front()).next)>::value));
}

/* Test: Input into a range using front_insert_n().
 * 
 * Everything in the input sequence that can be converted to the range's value
 * type should be read until either n items are read, or there is a
 * conversion error, an I/O error, or EOF. All elements read should be appended
 * to the back of the front sequence, in order of reading.
 */
TEST(FrontInsertN, Input)
{
  {
    auto r = std::deque<double>{};
    
    std::istringstream iss{"12.3 .34 1e5 -.1"};
    iss.imbue(std::locale::classic());
    
    EXPECT_TRUE(iss >> std::front_insert_n(r, 2));
    EXPECT_FALSE(iss.eof());
    EXPECT_FALSE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{2}, r.size());
    EXPECT_EQ(.34, r.at(0));
    EXPECT_EQ(12.3, r.at(1));
  }
  {
    auto r = std::deque<double>{};
    
    std::istringstream iss{"12.3 .34 1e5 -.1"};
    iss.imbue(std::locale::classic());
    
    EXPECT_FALSE(iss >> std::front_insert_n(r, 10));
    EXPECT_TRUE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(std::size_t{4}, r.size());
    EXPECT_EQ(-.1, r.at(0));
    EXPECT_EQ(1e5, r.at(1));
    EXPECT_EQ(.34, r.at(2));
    EXPECT_EQ(12.3, r.at(3));
  }
}
