# Utily
**Utily** is a header-only library using modern C++ features. From basic helper types to reflection, this library supplies all the ideal functions to **build robust and compile-time compatiable** code in C++.

In addition the library is **optimised for Windows, Emscripten**, and Linux too.

## API Contents

```c++
namespace Utily {
    /* Types & Data-Structures. */
    class Error;    // TOTEST
    class Result;   // TOTEST
    class StaticVector<T, S>;

    /* Operating-System Optimised. (including Emscripten) */
    class AsyncFileReader;    // TODO EMSCRIPTEN & LINUX & TOTEST

    /* Iterator Adaptors. */
    namespace Split {
        class ByElement;
        class ByElements;
        class ByRange;      // TODO & TOTEST
        class ByRanges;     // TODO & TOTEST
    }
    auto split(range, auto...); 

    /* Algorithms. */
    namespace TupleAlgo {
        void for_each(tuple, pred);         // TOTEST
        void copy(tuple, iter);             // TOTEST
        void transform(tuple, iter, pred);  // TODO & TOTEST
        auto reduce(tuple, init, pred);     // TODO & TOTEST
    }   

    namespace Reflection {
        auto get_name<T>(); // TODO & TOTEST
    }

    namespace Concepts {
        concept HasMoveConstructor<T>;
        concept HasMoveOperator<T>;
        concept HasCopyConstructor<T>;
        concept HasCopyOperator<T>;
        concept IsContiguousRange<T>;
        concept IsCallableWith<T, Param>;
        concept IsConstCallableWith<T, Param>;
    }
}

```

## API Usage

<details><summary><b>Utily::Error</b></summary>

Useful to flag basic errors. Prefer passing a `std::string_view`/`const char*` over a `std::string` as they're cheaper. 

```c++
Utily::Error error{"Bad input"};
std::cout << error.what(); // Bad input
```

---

</details>

<details><summary><b>Utily::Result</b></summary>

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

</details>

<details><summary><b>Utily::AsyncFileReader</b></summary>

An async file reader. Optimised for each supported system by using API calls. 

```c++
Utily::AsyncFileReader::push(STANFORD_BUNNY_PATH).on_error([](auto& e) { std::cerr << e.what(); });
Utily::AsyncFileReader::push(SMALL_TEXT_PATH).on_error([](auto& e) { std::cerr << e.what(); });

Utily::AsyncFileReader::wait_for_all();

// 3 MB
std::vector<char> bunny = Utily::AsyncFileReader::pop(STANFORD_BUNNY_PATH);
// 38 Bytes
std::vector<char> text  = Utily::AsyncFileReader::pop(SMALL_TEXT_PATH);
```

**Notes:** 
- On windows it is run on the main thread.
    *(Much faster than STL, and sometimes even faster than basic WIN32 as it is uses the Async WIN32 API.)*

---

</details>



<details><summary><b>Utily::Reflection</b></summary>

Basic type reflection using [`std::source_location`](https://en.cppreference.com/w/cpp/utility/source_location) avaliable since C++20.
```c++
struct Foo;

constexpr static auto name = Utily::Relfection::get_name<Foo>();

std::println("Name: {}", name); // Name: Foo
```

---

</details>

<details><summary><b>Utily::StaticVector</b></summary>

A stack based `std::vector` with a fixed capacity. Useful when you want to avoid heap allocations. 
```c++
Utily::StaticVector<int, 10> s_vector{1, 2, 3, 4};
```

---

</details>

<details><summary><b>Utily::Concepts</b></summary>

Just a collection of [concepts](https://en.cppreference.com/w/cpp/concepts) to restrict/narrow types for templated functions ontop of the STL.

---

</details>


<details><summary><b>Utily::Split</b></summary>

Subdividing ranges ('splitting') is so common and there's many slightly different ways we need to do it. Below are the iterator classes for each type of split.

**Utily::Split::ByElement**
```c++
std::string notes = " I use only the  Utily library . ";
// NOTE: std::string_view split-type for char arrays.
for(std::string_view word : Utily::SplitByElement(notes, ' ')) {
    std::cout << word << '-';
}
// I-use-only-the-Utily-library-.-
```

**Utily::Split::ByElements**
```c++
std::vector<int> nums = {1, 2, 3, 4, 5, 6};
// NOTE: std::span split-type for contigious non-char arrays.
for(std::span<const int> num : Utily::SplitByElements(notes, std::to_array({ 2, 4 })) {
    std::print("{}, " num)
}
// [1], [3], [5, 6],
```

### Utily::split

The `Utily::split` function will auto deduce which split iterator class you want to use. 
```c++
auto splitter1 = Utily::split("abcd"sv, 'b');
auto splitter2 = Utily::split("abcd"sv, 'b', 'd', 'c');

// decltype(splitter1) = Utily::SplitByElement<std::string_view>
// decltype(splitter2) = Utily::SplitByElements<std::string_view, 3, char>
```

---

</details>

<details><summary><b>Utily::TupleAlgo</b></summary>

Often we have a `std::tuple` we want to iterate over like an array. Unlike a typical array, each element in a `std::tuple` may have a distinct type, and we aim to handle each type with a tailored approach when we come across it.

**Utily::TupleAlgo::for_each**
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

**Utily::TupleAlgo::copy**
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

</details>

## Installation

<details><summary><b>Modern CMake</b></summary>



*this section is todo, but will explain how to use cmakes content fetching module.*

---

</details>

*It's header-only too, so you could just download `include/Utily/`. However, this won't auto-magically get the latest release unlike the other options.*
