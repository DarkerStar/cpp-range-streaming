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
test_obj := overwrite.o \
            back_insert.o \
            front_insert.o \
            insert.o \
            write_all.o \
            write_all_delimited.o

# The header being tested.
test_inc := ../include/rangeio \
						../include/stream-formatting-saver.hpp \
						../include/input.hpp \
						../include/overwrite.hpp \
						../include/back_insert.hpp \
						../include/front_insert.hpp \
						../include/insert.hpp \
						../include/output.hpp

# Need to get the include paths right, and might as well turning threading off
# for Google Test.
CPPFLAGS += -I../include -I. -DGTEST_HAS_PTHREAD=0

# Include Boost test only if requested.
ifdef HAVE_BOOST
test_obj += boost.o \
            iterators.o
endif

# Google test stuff.
gtest_inc := gtest/gtest.h
gtest_obj := gtest/gtest-all.o

# Test executable name.
test_exe := rangeio_test

# Restrict the suffixes to the ones used
.SUFFIXES:
.SUFFIXES: .cpp .cc .hpp .h .o

# Default target just makes the test program.
all : $(test_exe)

.PHONY : all

# Pretty standard build command.
$(test_exe) : $(gtest_obj) main.o $(test_obj)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

# Dependencies. No need to muck around with automatic dependencies.
main.o : main.cpp $(gtest_inc)

$(gtest_obj): $(gtest_inc)

$(test_obj) : $(test_inc) $(gtest_inc)

# check target builds the test program then runs it.
check : $(test_exe)
	./$(test_exe)

.PHONY : check

# Standard clean target.
clean :
	-@rm -f *.o
	-@rm -f gtest/*.o
	-@rm -f $(test_exe)

.PHONY : clean
