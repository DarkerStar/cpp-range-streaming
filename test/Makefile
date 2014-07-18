# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

CPPFLAGS += -I../include -I. -DGTEST_HAS_PTHREAD=0

# Restrict the suffixes to the ones used
.SUFFIXES:
.SUFFIXES: .cpp .h .hpp .o

all : stream_range_test

.PHONY : all

stream_range_test : main.o output.o input.o gtest-all.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

TEST_HEADERS := ../include/stream_range
GTEST_HEADER := gtest/gtest.h

main.o : main.cpp $(GTEST_HEADER)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c main.cpp

input.o : input.cpp $(TEST_HEADERS) $(GTEST_HEADER)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c input.cpp

output.o : output.cpp $(TEST_HEADERS) $(GTEST_HEADER)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c output.cpp

gtest-all.o : gtest/gtest-all.cc $(GTEST_HEADER)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c gtest/gtest-all.cc

check : stream_range_test
	./stream_range_test

.PHONY : check

clean :
	-@rm -f *.o
	-@rm -f stream_range_test

.PHONY : clean
