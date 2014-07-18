C++ range streaming proposal
============================

Version: 1.0



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

This proposal suggests the addition of new manipulator-like functions for
streaming ranges to or from streams. Reading a stream into a range is as simple
as:

```C++
// Overwrite each item in r with a value read from the stream, stopping
// immediately when bool(in) is false.
in >> std::stream_range(r);
````

Writing a range to a stream is just as easy:

```C++
// Write each item in r to the stream in order, stopping immediately when
// bool(out) is false.
out << std::stream_range(r);
```

You can even use delimiters - *real* delimiters, which only appear *between*
items - like this:

```C++
// Write each item in r to the stream in order - writing delimiter between
// each item - stopping immediately when bool(out) is false.
out << std::stream_range(r, delimiter);
```

Even a C++ newbie, so long as ze is even vaguely familiar with stream I/O, can
figure out what is going on here.

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
std::vector<int> v = {1, 4, 6};
std::cout << '(' << std::stream_range(v, ", ") << ')'; // Prints (1, 4, 6)
```

Or even more tersely:

```C++
std::cout << '(' << std::stream_range({ 1, 4, 6 }, ", ") << ')';

// or if you really want to use a vector...
std::cout << '(' << std::stream_range(std::vector<int>{ 1, 4, 6 }, ", ") << ')';
```

The range used can be any type that works as the range in a range-`for`
expression. The delimiter - which only applies to output - can be any type that
has an `operator<<` defined for output streams (the delimiter does not apply to
input streams). In all cases, no copies are made unless necessary (when the
range or the delimiter is passed as an rvalue, it is necessary to keep a copy
constructed by moving from the argument(s)).

In the table below:

*    `is` is an object of type `std::basic_istream<CharT, Traits>`
*    `os` is an object of type `std::basic_ostream<CharT, Traits>`
*    `lr` is a non-`const` lvalue reference to a range
*    `cr` is a possibly `const` lvalue reference to a range
*    `rr` is a non-`const` rvalue to a range range
*    `cd` is a possibly `const` lvalue or a `const` rvalue of a type that has
     `operator<<` defined for `std::basic_ostream<CharT, Traits>`.
*    `rd` is a non-`const` rvalue of a type that has
     `operator<<` defined for `std::basic_ostream<CharT, Traits>`.

| Expression                   | Expression result  | Effects                                                                                                                                                          |
| :--------------------------- | :----------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `is >> stream_range(lr)`     | reference to `is`  | Each element in `lr` is overwritten by a value read from `is`. Stops immediately when `bool(is)` is `false`.                                                     |
| `is >> stream_range(rr)`     | reference to `is`  | Each element in `rr` is overwritten by a value read from `is`. Stops immediately when `bool(is)` is `false`.                                                     |
| `os << stream_range(cr)`     | reference to `os`  | Each element in `cr` is written  to `os`. `cr` is not copied. Stops immediately when `bool(os)` is `false`.                                                      |
| `os << stream_range(rr)`     | reference to `os`  | `rr` is moved and stored internally, then each element is written to `os`. Stops immediately when `bool(os)` is `false`.                                         |
| `os << stream_range(cr, cd)` | reference to `os`  | Each element in `cr` is written to `os`, with `cd` written between each element. `cr` is not copied. Stops immediately when `bool(os)` is `false`.               |
| `os << stream_range(cr, rd)` | reference to `os`  | Each element in `cr` is written to `os`, with `rd` written between each element. `cr` is not copied. Stops immediately when `bool(os)` is `false`.               |
| `os << stream_range(rr, cd)` | reference to `os`  | `rr` is moved and stored internally, then each element is written to `os`, with `cd` written between each element. Stops immediately when `bool(os)` is `false`. |
| `os << stream_range(rr, rd)` | reference to `os`  | `rr` is moved and stored internally, then each element is written to `os`, with `rd` written between each element. Stops immediately when `bool(os)` is `false`. |

This proposal focuses on ranges supplied as is, rather than the "traditional"
C++ way: as a pair of iterators. The intention is that if you *do* wish to use
a pair of iterators, you can do this (assuming something like N3350 gets
accepted):

```C++
os << std::stream_range(std::make_range(begin, end));
os << std::stream_range(std::make_range(begin, end), delimiter);
```

However, for completeness, I have also included the following forms:

```C++
os << std::stream_iterator_range(begin, end);
os << std::stream_iterator_range(begin, end, delimiter);
```

Which are equivalent to what is shown above.



Installation
------------

The entire proposal is contained in a single header file. Just make sure it can
be found on the include path, and you can use it.

For more details, please see the file called “INSTALL”.



Possible future directions
--------------------------

`std::stream_range` works for input - for a range with `N` elements, it will
(attempt) to read `N` items from the input stream. That means you must know
in advance how many items there are in the input source. Unfortunately, it
also means that if there is a failure before all `N` elements are read, you
don't know which elements in the range were successfully read.

For more practical input situations, the following additional range streaming
functions might be handy:

```C++
std::stream_range_back_insert(Range&&)
std::stream_range_front_insert(Range&&)
std::stream_range_insert(Range&&, Iterator)
```

The basic `std::stream_range(Range&&)` as applied to input is essentially:

```C++
std::copy_n(std::istream_iterator<T>{in}, std::size(r), std::begin(r))
```

except that it will stop immediately whenever `bool(in)` is false. By analogy,
these other three functions, in turn, would be:

```C++
std::copy(std::istream_iterator<T>{in}, std::istream_iterator<T>{}, std::back_inserter(r))
std::copy(std::istream_iterator<T>{in}, std::istream_iterator<T>{}, std::front_inserter(r))
std::copy(std::istream_iterator<T>{in}, std::istream_iterator<T>{}, std::inserter(r, r.begin()))
```

Another handy extension might be to have these functions also take an
optional size argument, allowing them to read up to that many items, stopping
early if the stream goes into a bad state:

```C++
std::stream_range_back_insert(Range&&, Size)
std::stream_range_front_insert(Range&&, Size)
std::stream_range_insert(Range&&, Iterator, Size)
```

These functions would essentially be:

```C++
std::copy_n(std::istream_iterator<T>{in}, n, std::back_inserter(r))
std::copy_n(std::istream_iterator<T>{in}, n, std::front_inserter(r))
std::copy_n(std::istream_iterator<T>{in}, n, std::inserter(r, r.begin()))
```

except they would stop early if the input stream goes into a bad state.

These functions may be added in a future version of this library, once some
questions have been addressed.



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
