C++ range streaming proposal
============================

Version: 2.0



What is it?
-----------

This is a reference implementation for a proposed extension to the C++ standard
library to add facilities to make using ranges with input and output streams
easier.



Latest version
--------------

The latest release can be downloaded from the [github project downloads
page](https://github.com/DarkerStar/cpp-range-streaming/downloads).



Introduction
------------

Range `for` made ranges a first-class citizen in C++, but streaming the contents
of ranges to/from IOstreams is a clumsy and difficult operation. The canonical
way is to use the Algorithms library and stream iterators, but these are
difficult to use, and do not handle formatting or error-checking very well.

This proposal suggests a set of range I/O functions that return range I/O
objects, which can be used in conventional stream I/O expressions. For output,
this can be as simple as:

```C++
auto const a = std::array<int, 3>{1, 4, 6};
std::cout << std::write_all(a);
// Prints: "146"
```

Or with delimiters:

```C++
std::cout << "{ " << std::write_all({1, 4, 6}, ", ") << " }";
// Prints: "{ 1, 4, 6 }"
```

For input, four different standard operations are provided: overwriting, back
inserting, front inserting, and general inserting:

```C++
auto r = std::array<double, 3>{};
std::cin >> std::overwrite(r);
// Reads 3 double values from cin into r

auto v = std::vector<int>{};
std::cin >> std::back_insert(v);
// Reads ints from cin until a read failure,
// adding them to v via v.push_back()

auto l = std::list<std::string>{};
std::cin >> std::front_insert_n(l, 10);
// Reads up to 10 strings from cin, stopping if there is a read failure,
// adding them to l via l.push_front()
```

Formatting is supported properly:

```C++
std::cout << "{ " << setw(3) << setfill('_') <<
             std::write_all({1, 4, 6}, ", ") << " }";
// Prints: "{ __1, __4, __6 }"

auto iss = std::istringstream{"abcdef"};
auto v = std::vector<std::string>{};
iss >> setw(3) >> back_insert(v);
// v = { "abc", "def" }
```

You can get a lot of information about the previous read/write by capturing
the range I/O object and querying it:

```C++
auto a = std::array<int, 100>{};
auto p = overwrite(a);
std::cin >> p;

if (p.next != a.end())
{
  std::cerr << "There was an error reading.\n";
  std::cerr << "Only " << p.count << " values were read,\n";
  std::cerr << "and " << p.stored << " values were stored in a,\n";
  std::cerr << "which are: { " <<
    std::write_all(boost::make_iterator_range(a.begin(), p.next) ", ") << " }.\n";
}
```

For more information, check out the specification and the reference in the `doc`
directory.



Installation
------------

The entire proposal is contained in a set of header files. Just make sure they
can be found on the include path, and you can use it.

For more details, please see the file called “INSTALL”.



Possible future directions
--------------------------

See `doc/Ideas` for some ideas being considered.



Licensing
---------

The Google Test code is available under a [New BSD
License](http://opensource.org/licenses/BSD-3-Clause). For more details about
Google Test, see [the Google Test project
site](https://code.google.com/p/googletest/).

All other code and content in this package is licenced under the GPLv3, as
described in the COPYING file.

For more details, please see the file called "COPYING".



Contacts
--------

For contact information, check out the
[github project page](https://github.com/DarkerStar/cpp-range-streaming).
