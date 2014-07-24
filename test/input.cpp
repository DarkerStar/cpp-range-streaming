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
 * using input.
 * 
 * These tests are not meant to be exhaustive, merely illustrative.
 */

#include <algorithm>
#include <array>
#include <locale>
#include <sstream>
#include <string>

#include <stream_range>

#include "gtest/gtest.h"

/* Test: Input into a range.
 * 
 * The "basic" use of std::stream_range() with input streams, replaces each
 * element in the range by reading a new value from the input stream. Should
 * leave the stream in the same state it would be in if the values were read
 * by a manual loop.
 */
TEST(Input, Range)
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
  
  EXPECT_TRUE(iss >> std::stream_range(a));
  EXPECT_TRUE(std::equal(a.begin(), a.end(), expected.begin()));
  
  EXPECT_TRUE(iss >> d);
  EXPECT_EQ(6.6, d);
}

/* Test: Input into an rvalue range.
 * 
 * This is a handy use of std::stream_range() with input streams, where the
 * range being read into is a temporary that will be immediately discarded.
 * This basically means that values of the range's type will be read and parsed
 * as many times as the size of the range. So if the range is array<complex, 3>,
 * that means 3 complex number values will be read. You can check if the stream
 * state is good after this to confirm it all went well, even if you don't want
 * the values.
 * 
 * To test this feature, the value *after* the discarded values is tested.
 */
TEST(Input, Discarding)
{
  // The goal here is to stream in, parse, and discard 5 doubles, in order to
  // get to the 6th.
  auto const src = std::string{"1.1 2.2 3.3 4.4 5.5 6"};
  
  std::istringstream iss{src};
  iss.imbue(std::locale::classic());
  
  EXPECT_TRUE(iss >> std::stream_range(std::array<double, 5>{}));
  
  auto d = 0.0;
  EXPECT_TRUE(iss >> d);
  EXPECT_EQ(6, d);
}

/* Test: Preservation of formatting from element to element.
 * 
 * Whatever the formatting state at the beginning of the input of a streamed
 * range, it should be applied to every element in the range.
 */
TEST(Input, Formatting)
{
  {
    auto r = std::array<std::string, 3>{ "xx", "yy", "zz" };
    
    std::istringstream iss{"abcdefg"};
    iss.width(2);
    
    EXPECT_TRUE(iss >> std::stream_range(r));
    EXPECT_EQ("ab", r[0]);
    EXPECT_EQ("cd", r[1]);
    EXPECT_EQ("ef", r[2]);
    
    auto c = 'a';
    EXPECT_TRUE(iss >> c);
    EXPECT_EQ('g', c);
  }
  {
    auto r = std::array<int, 3>{};
    
    std::istringstream iss{"10 0x10 010"};
    iss.imbue(std::locale::classic());
    iss.setf(std::ios_base::hex, std::ios_base::basefield);
    
    EXPECT_TRUE(iss >> std::stream_range(r));
    EXPECT_EQ(0x10, r[0]);
    EXPECT_EQ(0x10, r[1]);
    EXPECT_EQ(0x10, r[2]);
  }
}
