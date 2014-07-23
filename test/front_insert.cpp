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

#include <stream_range>

#include "gtest/gtest.h"

/* Test: Verify the types associated with front_insert() are correct.
 * 
 * The return value of front_insert() is part of the specification - it must
 * return a properly-templated range_front_inserter<>. And *that* type should
 * have a count() member function that returns size_t.
 */
TEST(FrontInsert, Types)
{
  auto v = std::deque<double>{};
  auto const l = std::list<std::string>{};
  
  EXPECT_TRUE((std::is_same<decltype(std::front_insert(v)), std::range_front_inserter<decltype(v)>>::value));
  EXPECT_TRUE((std::is_same<decltype(std::front_insert(l)), std::range_front_inserter<decltype(l)>>::value));
  
  EXPECT_TRUE((std::is_same<decltype(std::front_insert(v, v.front())), std::range_front_inserter<decltype(v)>>::value));
  EXPECT_TRUE((std::is_same<decltype(std::front_insert(l, l.front())), std::range_front_inserter<decltype(l)>>::value));
  
  EXPECT_TRUE((std::is_same<decltype(std::front_insert(v).count()), std::size_t>::value));
  EXPECT_TRUE((std::is_same<decltype(std::front_insert(v, v.front()).count()), std::size_t>::value));
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
    
    EXPECT_EQ(r.size(), std::size_t{4});
    EXPECT_EQ(r.at(0), -.1);
    EXPECT_EQ(r.at(1), 1e5);
    EXPECT_EQ(r.at(2), .34);
    EXPECT_EQ(r.at(3), 12.3);
  }
  {
    auto r = std::list<int>{ 3, 4, 5 };
    
    std::istringstream iss{"2 1 0 x"};
    iss.imbue(std::locale::classic());
    
    EXPECT_FALSE(iss >> std::front_insert(r));
    EXPECT_FALSE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(r.size(), std::size_t{6});
    for (auto i = r.cbegin(); i != r.cend(); ++i)
      EXPECT_EQ(*i, std::distance(r.cbegin(), i));
    
    iss.clear();
    auto c = 'a';
    EXPECT_TRUE(iss >> c);
    EXPECT_EQ(c, 'x');
  }
}

/* Test: Error checking on range streaming using front_insert().
 * 
 * Everything in the input sequence that can be converted to the range's value
 * type should be read until either a conversion error, an I/O error, or EOF.
 * The number of elements read should be returned by the count() member
 * function of range_front_insert<Range>.
 */
TEST(FrontInsert, ErrorChecking)
{
  {
    auto r = std::deque<int>{ 3, 4, 5 };
    
    std::istringstream iss{"2 1 0 x"};
    iss.imbue(std::locale::classic());
    
    auto proxy_object = std::front_insert(r);
    EXPECT_EQ(proxy_object.count(), std::size_t{0});
    
    EXPECT_FALSE(iss >> proxy_object);
    EXPECT_FALSE(iss.eof());
    EXPECT_TRUE(iss.fail());
    EXPECT_FALSE(iss.bad());
    
    EXPECT_EQ(proxy_object.count(), std::size_t{3});
    
    EXPECT_EQ(r.size(), std::size_t{6});
    for (auto i = r.cbegin(); i != r.cend(); ++i)
      EXPECT_EQ(*i, std::distance(r.cbegin(), i));
    
    iss.clear();
    auto c = 'a';
    EXPECT_TRUE(iss >> c);
    EXPECT_EQ(c, 'x');
  }
}
