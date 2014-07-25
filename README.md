C++ range streaming proposal
============================

Version: 1.4.0



What is it?
-----------

This is a reference implementation for a proposed extension to the C++ standard
library to add facilities to make using ranges with input and output streams
easier.



Latest version
--------------

The latest release can be downloaded from the [github project downloads
page](https://github.com/DarkerStar/cpp-range-streaming/downloads).



Documentation
-------------

Currently, the canonical way to use ranges with input or output streams is to
use stream iterators and the copy algorithm. You read a range from an input
stream like this:

```C++
std::copy(std::istream_iterator<type>{in}, std::istream_iterator<type>{}, std::begin(r));
```

and you write a range to an output stream like this:

```C++
std::copy(std::begin(r), std::end(r), std::ostream_iterator<type>{out});
```

If you want to include delimiters between the items as you write them - for
example: spaces, newlines, or commas - you can use this form:

```C++
std::copy(std::begin(r), std::end(r), std::ostream_iterator<type>{out, ", "});
```

Unfortunately, this doesn't work the way most people would hope, because the
delimiter is simply printed after every element... including the last one. There
is a proposal that aims to fix this (N3581, N4007, N4066), which would look
something like this:

```C++
std::copy(std::begin(r), std::end(r), std::make_ostream_joiner(out, ", "));
```

This is good, and stream iterators in general are a powerful tool that should
not be maligned or discarded. However, for the *vast* majority of uses, their
advantages are outweighed by their clumsiness.

Consider the following code lifted straight out of N4066 (edited slightly to
make the namespace prefixes consistent):

```C++
std::vector<int> v = {1, 4, 6};
std::cout << "(";
std::copy(v.begin(), v.end(), std::make_ostream_joiner(cout, ", "));
std::cout << ")"; // Prints (1, 4, 6) as desired
```

I want to make clear that I do not wish to impugn the "ostream joiner" proposal
or stream iterators in general. However, I think every sane and rational
programmer should think it absurd and embarrassing that *that* kind of code is
the best we can do in C++ in the twenty-first century.

But aesthetics is only the tip of the iceberg. The current canonical form for
reading or writing sequences to or from streams has a number of issues:

1.  It is verbose. Even the most basic I/O stream operation on a range means a
    function call that probably tests the limits of your coding standards' line
    length limits.
2.  It is unclear. Distinguishing an input or output operation from other copy
    operations is hard. Hell, it's not even easy to tell input from output.
3.  It is semantically misleading. It is unnatural - if not straight up wrong -
    to think of input or output as "copying". No one calls `operator<<` or
    `operator>>` "copy operators".
4.  It doesn't look like any other input or output operations, making them hard
    to spot in code.
5.  It requires including *both* the iterator library (for the stream iterators)
    *and* the algorithms library (which is very heavy), just for a trivial
    input or output operation you could do with a one-line range-for (but, of
    course, writing such loops manually should be discouraged).
6.  For a C++ beginner to grasp what should be a simple operation, they have to
    be exposed to the algorithms library and function templates, iterators and
    the iterators library, stream iterators, and have to figure out what all is
    going on with all that stuff... which can be overwhelming to say the least.
7.  Worst of all: it doesn't even work all that well! Consider the output case:
    if something goes wrong with the output stream after the first operation
    (or even before!), and the stream goes into a fail state, `std::copy()`
    will merrily keep chugging along, blissfully unaware that it is just burning
    CPU cycles as it reads each element of the range and tries to print it. If
    the range is a few million elements, that could be substantial time wasted.
    Or consider the case of streaming in N items - naturally you would use
    `std::copy_n()`, right? Well, that's all fine and good if there are no
    problems, but if there are any errors in reading or formatting... game over;
    you're getting a container full of junk, and you'll have no idea where
    things went wrong.

This proposal suggests the addition of new manipulator-like functions (but
*not* manipulators) for streaming ranges to or from streams.

### Output

Writing the contents of a range `r` to a stream is as simple as:

```C++
out << std::stream_range(r);
```

If you want delimiters between each value written:

```C++
out << std::stream_range(r, '\n');
```

And because this structure is natural for I/O, everything else becomes easier.
Recall the code from N4066:

```C++
std::vector<int> v = {1, 4, 6};
std::cout << "(";
std::copy(v.begin(), v.end(), std::make_ostream_joiner(cout, ", "));
std::cout << ")"; // Prints (1, 4, 6) as desired
```

With range streaming, that becomes:

```C++
auto v = std::vector<int>{1, 4, 6};
std::cout << '(' << std::stream_range(v, ", ") << ')'; // Prints (1, 4, 6)
```

Or even more tersely:

```C++
std::cout << '(' << std::stream_range({ 1, 4, 6 }, ", ") << ')';

// or if you really want to use a vector...
std::cout << '(' << std::stream_range(std::vector<int>{ 1, 4, 6 }, ", ") << ')';
```

Unlike with the copy/stream-iterator model, stream errors are detected and
handled properly. The stream's conversion to `bool` is checked after each write
(attempt), and if it is `false`, the function stops attemping to write and
returns immediately. You can query the number of elements that were written
with the `count()` function:

```C++
auto p = std::stream_range(r);
out << r;
std::cout << p.count() << " items were written of " << r.size() << " in the range.";
```

Also unlike with the copy/stream-iterator model, formatting is handled correctly.
With the following code:

```C++
std::cout.fill('_');
std::cout.setf(std::ios_base::hex, std::ios_base::basefield);

std::cout << 'x' << std::setw(5) << std::stream_range(r, '|') << 'x';
```

Then the output will be (dependent on the content of `r`):

```
// r = {} (r is empty)
x_____x
// r = { 25 }
x___19x
// r = { 6, 24, 151 }
x____6|___18|___97x
```

### Input

Input comes in 4 different forms:

*   Overwriting.
*   Back inserting.
*   Front inserting.
*   General inserting.

The latter three mimic the behaviour of the existing `back_insert_iterator`,
`front_insert_iterator`, and  `insert_iterator`, respectively.

The following reads 4 `int` values from `std::cin`, overwriting the contents
of `r`:

```C++
auto r = std::array<int, 4>{};
std::cin >> std::overwrite(r);
```

The following code reads double values from `infile` until `EOF` or a parse
error, and uses `push_back()` to add the values to `r`:

```C++
auto r = std::deque<double>{};
std::cin >> std::back_insert(r);
```

If you wish to use `push_front()` instead, just replace `back_insert()` with
`front_insert()`:

```C++
auto r = std::deque<double>{};
std::cin >> std::front_insert(r);
```

If you wish to insert into any place in `r`, use `insert()` with an iterator to
the spot to insert before:

```C++
auto r = std::vector<int>{ 0, 1 };
std::cin >> std::insert(r, std::next(std::begin(r), 1));
```

Error checking works, too:

```C++
auto r = std::array<int, 100>{};
auto p = std::overwrite(r);
if (!(std::cin >> p))
  std::cerr << "Error: only " << p.count() << " items were read successfully.";
```

Formatting also works:

```C++
auto iss = std::istringstream{"abcdef"};
auto r = std::vector<std::string>{};
iss >> std::setw(2) >> std::back_insert(r);
// r is { "ab", "cd", "ef" }
```

For more details see the `doc` directory.



Installation
------------

The entire proposal is contained in a single header file. Just make sure it can
be found on the include path, and you can use it.

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
