# Utily
**Utily** is a utility library using modern C++ features. From basic helper types to reflection, this library supplies all the ideal functions to build clean, robust, and performant code in C++.

## Contents
- [x] = Implemented. ✅ = Tested.

> **Error Handling**
> - [x] [Error](#error)
> - [x] [Result](#result) ✅

> **Data Stuctures**
> - [ ] [StaticVector](#staticvector)
> - [ ] ParallelVector *('SoA' aka Structure of Arrays)*

> **Type Info**
> - [ ] [Reflection](#reflection) 
> - [ ] [Concepts](#concepts) 

> **Algorithms**
> - [ ] [Split](#split)
>   - [x] SplitByElement ✅
>   - [x] SplitByElements ✅
>   - [ ] SplitByRange
>   - [ ] SplitByRanges
> 
> - [ ] [TupleAlgo](#tuplealgo)
>   - [x] TupleAlgo::for_each
>   - [x] TupleAlgo::copy 
>   - [ ] TupleAlgo::reduce
>   - [ ] TupleAlgo::transform  

## Examples
### Error 
Useful to flag basic errors. Prefer passing a `std::string_view`/`const char*` over a `std::string` as they're cheaper. 
```c++
Utily::Error error{"Bad input"};
std::cout << error.what(); // Bad input
```
---
### Result
Useful return type for when things can fail. Its pretty much a wrapper around [`std::variant`](https://en.cppreference.com/w/cpp/utility/variant) specifying the good and bad types. The goal is to be less hassle than [`std::expected`](https://en.cppreference.com/w/cpp/utility/expected). 

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
Basic type reflection using [`std::source_location`](https://en.cppreference.com/w/cpp/utility/source_location) avaliable since C++20.
```c++
struct Foo;

constexpr static auto name = Utily::Relfection::get_name<Foo>();

std::println("Name: {}", name); // Name: Foo
```
---
### StaticVector
A stack based `std::vector` with a fixed capacity. Useful when you want to avoid heap allocations. 
```c++
Utily::StaticVector<int, 10> s_vector{1, 2, 3, 4};
```
---
### Concepts
Just a collection of [concepts](https://en.cppreference.com/w/cpp/concepts) to restrict/narrow types for templated functions ontop of the STL.

--- 
### Split
Subdividing ranges ('splitting') is so common and there's many slightly different ways we need to do it. The `Utily::split` function lets you do it all. 

**split**
```c++
// auto = Utily::SplitByElement<std::string_view>
auto splitter1 = Utily::split("abcd"sv, 'b');

// auto = Utily::SplitByElements<std::string_view, 3, char>
auto splitter2 = Utily::split("abcd"sv, 'b', 'd', 'c');
```
**SplitByElement**
```c++
std::string notes = " I use only the  Utily library . ";
// NOTE: std::string_view split-type for char arrays.
for(std::string_view word : Utily::SplitByElement(notes, ' ')) {
    std::cout << word << '-';
}
// I-use-only-the-Utily-library-.-
```

**SplitByElements**
```c++
std::vector<int> nums = {1, 2, 3, 4, 5, 6};
// NOTE: std::span split-type for contigious non-char arrays.
for(std::span<const int> num : Utily::SplitByElements(notes, std::to_array({ 2, 4 })) {
    std::print("{}, " num)
}
// [1], [3], [5, 6],
```
---

### TupleAlgo
Often we have a `std::tuple` we want to iterate over like an array. Unlike a typical array, each element in a `std::tuple` may have a distinct type, and we aim to handle each type with a tailored approach when we come across it.

**TupleAlgo::for_each**
```c++
struct Print
{
    auto operator()(int a) {
        std::cout << a << ' ';
    }
    auto operator()(bool a) {
        std::cout << (a) ? "true" : "false"  << ' ';
    }
};

// compiles and outputs: "1 true 2 false "
Utily::TupleAlgo::for_each(std::make_tuple(1, true, 2, false), Print);

// fails to compile: 
// "static assertion failed: Predicate must be callable with all tuple element types"
Utily::TupleAlgo::for_each(std::make_tuple(1, true, 2, "hi"sv), Print);

```

**TupleAlgo::copy**
```c++
/*
    This gives the compiler a ton of information so 
    the generated asm is typically super efficient.
*/
template<typename T, typename... Args>
constexpr auto to_array(Args&&... args)
{
    auto array = std::array<T, sizeof...(Args)>{};
    Utily::TupleAlgo::copy(std::forward_as_tuple(args...), array.begin());
    return array;
}
```
---