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

# The test objects - this is the only part that should change from time to time.
test_obj := input.o \
            output.o \
            back_insert.o \
            front_insert.o \
            iterators.o

# The header being tested.
test_inc := ../include/stream_range

# Need to get the include paths right, and might as well turning threading off
# for Google Test.
CPPFLAGS += -I../include -I. -DGTEST_HAS_PTHREAD=0

# Include Boost test only if requested.
ifdef HAVE_BOOST
test_obj += boost.o
endif

# Google test stuff.
gtest_inc := gtest/gtest.h
gtest_obj := gtest/gtest-all.o

# Restrict the suffixes to the ones used
.SUFFIXES:
.SUFFIXES: .cpp .cc .hpp .h .o

# Default target just makes the test program.
all : stream_range_test

.PHONY : all

# Pretty standard build command.
stream_range_test : main.o $(test_obj) $(gtest_obj)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

# Dependencies. No need to muck around with automatic dependencies.
main.o : main.cpp $(gtest_inc)

$(gtest_obj): $(gtest_inc)

$(test_obj) : $(test_inc) $(gtest_inc)

# check target builds the test program then runs it.
check : stream_range_test
	./stream_range_test

.PHONY : check

# Standard clean target.
clean :
	-@rm -f *.o
	-@rm -f gtest/*.o
	-@rm -f stream_range_test

.PHONY : clean
