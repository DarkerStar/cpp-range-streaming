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
 * using the range adaptors in Boost.Range. Both the versions with and without
 * delimiters are tested.
 * 
 * If you don't have Boost, or if you want to disable testing with Boost,
 * just set a shell variable NO_BOOST before running make (for example: by
 * running the command "NO_BOOST=1 make check", without the quotes).
 * 
 * These tests are not meant to be exhaustive, merely illustrative.
 */

#include <sstream>
#include <vector>

#include <rangeio>

#include <boost/range/adaptors.hpp>

#include "gtest/gtest.h"

/* 
 * Some extra types used in the tests are contained in this namespace.
 */
namespace {

struct is_even { auto operator()(int x) const -> bool { return x % 2 == 0; } };
struct is_odd  { auto operator()(int x) const -> bool { return x % 2 == 1; } };

} // anonymous namespace

/* Test: Streaming an adapted range (using Boost.Range's range adaptors).
 */
TEST(Boost, AdaptedOutput)
{
  auto const v = std::vector<int>{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  
  using namespace boost::adaptors;
  
  {
    std::ostringstream oss{};
    
    EXPECT_TRUE(oss << std::write_all(v | reversed | filtered(is_even{})));
    EXPECT_EQ("108642", oss.str());
  }
  
  {
    std::ostringstream oss{};
    
    EXPECT_TRUE(oss << std::write_all(v | reversed | filtered(is_odd{}), ", "));
    EXPECT_EQ("9, 7, 5, 3, 1", oss.str());
  }
}
