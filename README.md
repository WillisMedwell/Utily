# Utily

![Emscripten SDK Build](https://github.com/WillisMedwell/Utily/actions/workflows/emscripten.yml/badge.svg)
![Clang Build](https://github.com/WillisMedwell/Utily/actions/workflows/clang.yml/badge.svg)
![GCC Build](https://github.com/WillisMedwell/Utily/actions/workflows/gcc.yml/badge.svg)

**Utily** is a library using modern C++ features. From basic helper types to reflection, this library supplies all the ideal functions to **build robust** software in C++.

In addition the library is **optimised for Windows, Emscripten**, and Linux too.



## Library Contents

```c++
namespace Utily {
    class Error;    
    class Result;
    class StaticVector<T, S>;                                    // perf as *good as std::array on Clang & GCC. 
    class TypeErasedVector;
    class InlineArrays {                                        
        static alloc_uninit<T1, T2,...>(s1, s2,...);
        static alloc_default<T1, T2, ...>(s1, s2, ...);
        static alloc_copy<T1, T2>(R1 range1, R2 range2);
    };
    struct Reflection {
        get_type_name<T>();
    };
    namespace Simd {                                             // Simd optimised algo's. Use flag "-mtune=native"
        iter find(begin, end, value);                            // ~ x5 faster than std::find for char searching.
        iter find_first_of(begin, end, value_begin, value_end);  // ~ x10 faster than std::find_first_of for char searching.
    }
    class FileReader {
        static load_entire_file(path)                            // ~ x10 faster than using the STL on windows
    }
    namespace Split {
        class ByElement;
        class ByElements;                                      
    }
    auto split(range, auto...); 
    namespace TupleAlgo {
        void for_each(tuple, pred);
        void copy(tuple, iter);
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
/*
    For more info on performance, check out the actions tab
    and have a look at the `Run Test` section to check for suitability. 
*/
```

## Examples

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

<details><summary><b>Utily::StaticVector</b></summary>

A stack based `std::vector` with a fixed capacity. Useful when you want to avoid heap allocations. 
```c++
Utily::StaticVector<int, 10> s_vector{1, 2, 3, 4};
```

---

</details>

<details><summary><b>Utily::TypeErasedVector</b></summary>

A vector with no compile time enfored type. Access is checked in debug mode at runtime using Reflection.
Useful for on the fly composing of types.
```c++
// cannot resize or push back if the underlying_type is not set.
auto vector = Utily::TypeErasedVector {};
vector.set_underlying_type<float>();

// these operations will assert false in debug mode.
vector.emplace_back<int>(1);       
vector.emplace_back<double>(1);

// these will be valid.
vector.push_back<float>(0.0f);
vector.emplace_back<float>(1.0f);

// as_span<T> will assert if T != Underlying.
for (float& v : vector.as_span<float>()) {
    std::cout << v << ' ';
}
```

---

</details>

<details><summary><b>Utily::InlineArrays</b></summary>
An allocator that will pack arrays together for optimal memory access.

```C++
using ReturnType = std::tuple<
    std::unique_ptr<byte[]>, 
    std::span<int>, 
    std::span<bool>
>;
// the spans elements will have uninitialised memory so be careful.
ReturnType uninitialised = Utily::InlineArrays::alloc_uninit<int, bool>(5, 10);
```
```C++
// the spans elements will be defaulted constructed. 
// also structured bindings are good.
auto [data, ints, bools] = Utily::InlineArrays::alloc_default<int, bool>(10, 10);
```
```C++
auto a = std::to_array<int>({1, 2, 3, 4});
auto b = std::vector<bool>{true, false, true, false};
auto c = Utily::StaticVector<char, 10>{'a', 'b', 'c'};

auto [data1, ints1, bools1, chars1] = Utily::InlineArrays::alloc_copy<int, bool, char>(a, b, c);
// can also deduce types from the ranges.
auto [data2, ints2, bools2, chars2] = Utily::InlineArrays::alloc_copy(a, b, c);    
```

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

<details><summary><b>Utily::Simd</b></summary>

Simd optimised operations for supported algorithms. Mostly char searching at the moment.

```c++

const auto DELIMS = std::string_view { "azxy" };
const auto STRING = std::string { "hello world! This is a sentenze" };

```
```C++
auto iter = Utily::Simd::find(STRING.begin(), STRING.end(), DELIMS.front());
```
```C++
auto iter = Utily::Simd::find_first_of(
    STRING.begin(), STRING.end(), 
    DELIMS.begin(), DELIMS.end()
);
```

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

## Performance

<details><summary><b>Simd128::Char</b></summary>

**Windows MSVC** 

| String Operation        | Std     | StringZilla | Utily       |
| ----------------------- | ------- | ----------- | ----------- |
| find                    | 13.2 ns | 109 ns      | **11.3 ns** |
| find_first_of           | 837 ns  |             | **46.2 ns** |
| find_substring(char[4]) | 192 ns  | 609 ns      | **58.4 ns** |
| find_substring(char[8]) | 186 ns  | 1842 ns     | **65.6 ns** |

*NOTE: StringZilla's method for SIMD seems be ignored by MSVC*

**Clang (Server)** 

| String Operation        | Std    | StringZilla | Utily      |
| ----------------------- | ------ | ----------- | ---------- |
| find                    | 205 ns | **124 ns**  | 203 ns     |
| find_first_of           | 630 ns |             | **324 ns** |
| find_substring(char[4]) | 243 ns | 646 ns      | **205 ns** |
| find_substring(char[8]) | 254 ns | 1250 ns     | **207 ns** |


**Emscripten**

| String Operation        | Std     | StringZilla | Utily       |
| ----------------------- | ------- | ----------- | ----------- |
| find                    | 405 ns  |             | **22.3 ns** |
| find_first_of           | 1555 ns |             | **136 ns**  |
| find_substring(char[4]) | 1110 ns |             | **196 ns**  |
| find_substring(char[8]) | 938 ns  |             | **574 ns**  |

*NOTE: Only 128bit vector operations supported. *

</details>

## Get Started

<details><summary><b>Modern CMake</b></summary>
Using Modern cmake features means that we can use CMake as a dependency manager relatively easily.
 
```CMake
include(FetchContent)

FetchContent_Declare(
    Utily
    GIT_REPOSITORY https://github.com/WillisMedwell/Utily.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(Utily)

target_link_libraries(${PROJECT_NAME} PRIVATE Utily::Utily)
```

In the future, I would like to have Utily:: supported by package managers like vcpkg and conan.

---

</details>

<details><summary><b>Building Tests and Benchmarks</b></summary>

I don't want my users to be wasting time by building the tests and benchmarks.

As such, to build these tests and benchmarks you will need to have both [benchmark](https://github.com/google/benchmark) & [gtest](https://github.com/google/googletest) as 'findable' packages using cmake's `find_package`


---

</details>


<details><summary><b>Conditions</b></summary>

I think its fair to recognise the use of other people's libraries and code; I strive to do my best to recognise other people's contribution and work. 

If you use this library in a public repo, I would appreciate a link to my repo to let others know!
```md
This project makes use of the [Utily](https://github.com/WillisMedwell/Utily) library *created by Willis Medwell.*
```

And let me know what you create, I'm always keen to see other people's amazing work!


---

</details>


