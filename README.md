# Utily
**Utily** is a utility library using modern C++ features. From basic helper types to reflection, this library supplies all the ideal functions to build clean, robust, and performant code in C++.

## Contents
> **Error Handling**
> - [x] [Error](#error)
> - [x] [Result](#result)

> **Data Stuctures**
> - [ ] [StaticVector](#staticvector)
> - [ ] ParallelVector *('SoA' aka Structure of Arrays)*

> **Type Info**
> - [ ] [Reflection](#reflection) 
> - [ ] [Concepts](#concepts)

> **Algorithms**
> - [ ] [Split](#split)
>   - [x] SplitByElement
>   - [ ] SplitByElements
>   - [ ] SplitByRange
>   - [ ] SplitByRanges 

## Examples
### Error 
Useful to flag basic errors. Prefer passing a `std::string_view`/`const char*` over a `std::string` as they're cheaper. 
```c++
Utily::Error error{"Bad input"};
std::cout << error.what(); // Bad input
```
---
### Result
Useful return type for when things can fail. Its pretty much a wrapper around [std::variant](https://en.cppreference.com/w/cpp/utility/variant) specifying the good and bad types. The goal is to be less hassle than [std::expected](https://en.cppreference.com/w/cpp/utility/expected). 

```c++
constexpr Utily::Result<int, Utily::Error> do_thing()
{
    if(is_bad) {
        return Utily::Error{"Not good."};
    } 
    return 1;
}
```
Can pass callables for clean handling.
```c++ 
auto print_value = [](int value) { std::println("Good value {}", value); };
auto print_error = [](Utily::Error error) { std::println("bad value {}", error.what()); };

// Style 1.
if(auto result = do_thing(); result.has_value()) {
    result.on_value(print_value);
} else if(result.has_error()) {
    result.on_error(print_error);
}

// Style 2.
auto result = do_thing()
    .on_value(print_value)
    .on_error(print_error);

// Style 3.
auto result = do_thing()
    .on_either(print_value, print_error);
```
---
### Reflection
Basic type reflection using [std::source_location](https://en.cppreference.com/w/cpp/utility/source_location) avaliable since C++20.
```c++
struct Foo;

constexpr static auto name = Utily::Relfection::get_name<Foo>();

std::println("Name: {}", name); // Name: Foo
```
---
### StaticVector
A stack based vector with a fixed capacity. Useful when you want to avoid heap allocations. 
```c++
Utily::StaticVector<int, 10> s_vector{1, 2, 3, 4};
```
---
### Concepts
Just a collection of [concepts](https://en.cppreference.com/w/cpp/concepts) to restrict/narrow types for templated functions ontop of the STL.

--- 
### Split
Subdividing ranges ('splitting') is so common and there's many slightly different ways we need to do it. The `Utily::split` function lets you do it all. 

**SplitByElement**
```c++
std::string notes = " I use only the  Utily library . ";
// NOTE: std::string_view split-type for char arrays.
for(std::string_view word : Utily::split(notes, ' ')) {
    std::cout << word << '-';
}
// I-use-only-the-Utily-library-.-
```

**SplitByElements**
```c++
std::string notes = " I use only the  Utily library . ";
// NOTE: std::string_view split-type for char arrays.
for(std::string_view word : Utily::split(notes, ' ', '.')) {
    std::cout << word << '-';
}
// I-use-only-the-Utily-library-
```