# zstr.h

`zstr.h` is a modern, single-header string library for C projects. Designed to mimic the architecture of C++ `std::string` (specifically Small String Optimization and Views), it offers a safe, convenient API while remaining pure C.

It also includes a robust **C++17 wrapper**, allowing you to use it as a lightweight, drop-in string class in mixed codebases.

## Features

* **Small String Optimization (SSO)**: Strings shorter than 23 bytes are stored directly on the stack (inside the struct), avoiding heap allocation entirely.
* **View Semantics**: Distinct `zstr` (owning) and `zstr_view` (non-owning) types allow for zero-copy slicing and parsing.
* **Safe & Auto-Growing**: Buffer overflows are handled by automatic reallocation.
* **C++ Support**: Includes a full C++ class wrapper (`z_str::string`) with RAII, move semantics, and `std::string_view` compatibility.
* **UTF-8 Aware**: Includes helpers for UTF-8 validation and rune counting.
* **Zero Dependencies**: Only standard C headers used.

## Installation

`zstr` is a concrete type, so setup is straightforward.

1.  Copy `zstr.h` (and `zcommon.h` if separated) into your project's include directory.
2.  Include it in your code.

## Usage: C

```c
#include <stdio.h>
#include "zstr.h"

int main(void)
{
    // Initialize (SSO avoids malloc here).
    zstr s = zstr_lit("Hello");

    // Concatenate using printf-style formatting.
    zstr_fmt(&s, ", %s!", "World");

    // Access C-string safely.
    printf("%s\n", zstr_cstr(&s)); // "Hello, World!"

    // Create a zero-copy view of the first 5 chars.
    zstr_view v = zstr_sub(zstr_as_view(&s), 0, 5);

    // Cleanup.
    zstr_free(&s);
    return 0;
}
```

## Usage: C++

The library detects C++ compilers automatically. To avoid naming collisions with the C struct `zstr`, the C++ classes live in the **`z_str`** namespace.

```cpp
#include <iostream>
#include "zstr.h"

int main()
{
    // RAII handles memory automatically (Constructor/Destructor).
    z_str::string s = "Hello";

    // Operator overloads.
    s += " World!";

    // Modern C++17 string_view support.
    z_str::view v = s; 

    // Static formatting helper (Safety Warning: Only pass POD types!).
    z_str::string log = z_str::string::fmt("ID: %d", 42);

    std::cout << s << std::endl; // "Hello World!"
    
    return 0;
}
```

## API Reference (C)

### Initialization & Creation

| Function | Description |
| :--- | :--- |
| `zstr_init()` | Returns an empty string `{0}`. |
| `zstr_from(const char *s)` | Creates a new `zstr` from a standard C-string. |
| `zstr_from_len(const char *p, size_t len)` | Creates a `zstr` from a buffer and explicit length. |
| `zstr_lit("literal")` | Optimized macro for string literals (calculates length at compile time). |
| `zstr_dup(const zstr *s)` | Creates a deep copy of an existing `zstr`. |
| `zstr_with_capacity(size_t cap)` | Creates an empty string with pre-allocated heap capacity. |
| `zstr_read_file(path)` | Reads an entire file into a new `zstr`. Returns empty on failure. |
| `zstr_own(ptr, len, cap)` | Takes ownership of a raw `malloc`'d buffer. |

### Memory Management

| Function | Description |
| :--- | :--- |
| `zstr_free(s)` | Frees the string if it is on the heap and resets it to empty. |
| `zstr_clear(s)` | Sets length to 0 but preserves capacity. |
| `zstr_reserve(s, cap)` | Ensures the string has space for at least `cap` bytes. |
| `zstr_shrink_to_fit(s)` | Reduces heap usage to fit the length (or moves back to SSO). |
| `zstr_take(s)` | Returns the raw `malloc`'d pointer and resets the `zstr` (caller must free). |

### Modification

| Function | Description |
| :--- | :--- |
| `zstr_cat(s, str)` | Appends a C-string to the end. |
| `zstr_cat_len(s, ptr, len)` | Appends a raw buffer of known length. |
| `zstr_push(s, char)` | Appends a single character (alias for `zstr_push_char`). |
| `zstr_pop_char(s)` | Removes and returns the last character. |
| `zstr_fmt(s, fmt, ...)` | Appends a formatted string (printf-style). |
| `zstr_join(arr, n, delim)` | Joins an array of strings into a new `zstr`. |
| `zstr_trim(s)` | Removes leading and trailing whitespace in-place. |
| `zstr_to_lower(s)` | Converts to lowercase in-place (ASCII only). |
| `zstr_to_upper(s)` | Converts to uppercase in-place (ASCII only). |
| `zstr_replace(s, old, new)` | Replaces all occurrences of `old` with `new`. |

### Accessors & Helpers

| Function | Description |
| :--- | :--- |
| `zstr_len(s)` | Returns the current length (excluding null terminator). |
| `zstr_data(s)` | Returns a pointer to the mutable data buffer. |
| `zstr_cstr(s)` | Returns a `const char*` safe for C APIs. |
| `zstr_is_empty(s)` | Returns `true` if the string length is 0. |
| `zstr_is_long(s)` | Returns `true` if the string is currently heap-allocated. |

### Comparison & Search

| Function | Description |
| :--- | :--- |
| `zstr_eq(a, b)` | Returns `true` if strings are equal (faster than `strcmp`). |
| `zstr_eq_ignore_case(a, b)` | Returns `true` if strings are equal (case-insensitive, ASCII only). |
| `zstr_cmp(a, b)` | Standard `strcmp` behavior for zstr objects. |
| `zstr_find(s, needle)` | Returns index of first occurrence or -1 if not found. |
| `zstr_contains(s, needle)` | Returns `true` if the string contains the substring. |
| `zstr_starts_with(s, pre)` | Checks if string starts with prefix. |
| `zstr_ends_with(s, suf)` | Checks if string ends with suffix. |

### Views & Slices (Zero-Copy)

| Function | Description |
| :--- | :--- |
| `ZSV("lit")` | Macro to create a `zstr_view` from a literal. |
| `zstr_as_view(s)` | Returns a `zstr_view` covering the whole string. |
| `zstr_view_from(cstr)` | Creates a view from a C-string. |
| `zstr_from_view(v)` | Converts a view back into an owning `zstr` (allocates). |
| `zstr_sub(v, start, len)` | Returns a view slice (substring) without copying memory. |
| `zstr_view_eq(v, cstr)` | Checks if view equals a C-string. |
| `zstr_view_eq_view(a, b)` | Checks if two views are equal. |
| `zstr_view_starts_with(v, pre)`| Checks if view starts with prefix. |
| `zstr_view_ends_with(v, suf)` | Checks if view ends with suffix. |
| `zstr_view_lstrip(v)` | Returns view with leading whitespace removed. |
| `zstr_view_rstrip(v)` | Returns view with trailing whitespace removed. |
| `zstr_view_trim(v)` | Returns view with both ends trimmed. |
| `zstr_view_to_int(v, out)` | Parses an integer from a view (like `atoi`). |

### UTF-8 Support

| Function | Description |
| :--- | :--- |
| `zstr_is_valid_utf8(s)` | Validates string is strict UTF-8 (rejects overlongs/surrogates). |
| `zstr_count_runes(s)` | Counts the number of actual UTF-8 Runes (not bytes). |
| `zstr_next_rune(ptr)` | Decodes next rune and advances pointer. Returns `0xFFFD` on error. |

### Iteration (Splitting)

| Function | Description |
| :--- | :--- |
| `zstr_split_init(src, delim)` | Initializes a split iterator (`zstr_split_iter`). |
| `zstr_split_next(it, out)` | Advances iterator and populates `out` (view) with the next part. |

### Extensions (Experimental)

If you are using a compiler that supports `__attribute__((cleanup))` (like GCC or Clang), you can use the **Auto-Cleanup** extension.

| Macro | Description |
| :--- | :--- |
| `zstr_autofree` | Declares a variable that automatically calls `zstr_free` when it leaves scope. |

**Example:**

```c
void log_status()
{
    zstr_autofree msg = zstr_from("Status: ");
    zstr_cat(&msg, "OK");
    // msg is freed automatically here.
}
```

## API Reference (C++)

The C++ wrapper is defined in the **`z_str`** namespace to avoid collisions with the C struct. It strictly adheres to RAII principles and is designed to drop into existing C++11/17 projects.

### `class z_str::string`

An owning string class that internally manages a `zstr` struct.

**Constructors & Management**

| Method | Description |
| :--- | :--- |
| `string()` | Default constructor (empty). |
| `string(const char*)` | Construct from C-string. |
| `string(std::string_view)` | Construct from C++17 string view. |
| `own(ptr, len, cap)` | **Static**. Wraps an existing `malloc`'d buffer without copying. |
| `from_file(path)` | **Static**. Reads entire file into a string. |
| `release()` | Returns the raw `char*` and empties the object. **Caller must `free()`**. |

**Access & Iterators**

| Method | Description |
| :--- | :--- |
| `c_str()`, `data()` | Returns `const char*`. |
| `size()`, `length()` | Returns length in bytes. |
| `capacity()` | Returns current allocated capacity. |
| `is_empty()` | Returns `true` if length is 0. |
| `operator[]` | Mutable/Const access to character at index. |
| `front()`, `back()` | Access first/last character. |
| `begin()`, `end()` | Standard iterators (pointers) compatible with STL algorithms. |

**Modification**

| Method | Description |
| :--- | :--- |
| `append(s)` / `+=` | Appends C-string, character, or other `z_str::string`. |
| `push_back(c)` | Appends a single char. |
| `pop_back()` | Removes and returns the last char. |
| `replace(old, new)` | Replaces all occurrences of string `old` with `new`. |
| `to_lower()`, `to_upper()`| In-place case conversion (ASCII). |
| `trim()` | In-place whitespace removal. |
| `clear()` | Sets length to 0 (capacity remains). |
| `fmt(fmt, ...)` | **Static**. Creates a string via printf formatting. <br>**Warning:** Pass only POD types (`int`, `char*`), not C++ objects. |

**Search & Utilities**

| Method | Description |
| :--- | :--- |
| `find(needle)` | Returns index of substring or -1. |
| `contains(needle)` | Returns `true` if substring exists. |
| `starts_with(s)` | Returns `true` if string starts with `s`. |
| `ends_with(s)` | Returns `true` if string ends with `s`. |
| `split(delim)` | Returns a `split_iterable` for use in range-based for loops. <br>**Safety:** Deleted for r-values (temporaries) to prevent dangling views. |
| `rune_count()` | Returns the number of UTF-8 code points. |
| `is_valid_utf8()` | Returns `true` if the string contains valid UTF-8. |

---

### `class z_str::view`

A lightweight, non-owning wrapper around `zstr_view`. Compatible with `std::string_view` (C++17).

| Method | Description |
| :--- | :--- |
| `view(string&)` | Implicit conversion from `z_str::string`. |
| `sub(start, len)` | Returns a new view slice. |
| `lstrip()`, `rstrip()`, `trim()` | Returns a new view with whitespace removed. |
| `to_int(out)` | Parses view to integer. Returns `true` on success. |
| `starts_with`, `ends_with` | Predicate checks. |
| `operator==` | Compares with `view`, `string`, or `const char*`. |

## Memory Management

By default, `zstr.h` uses the standard C library functions (`malloc`, `realloc`, `free`).

However, you can override these to use your own memory subsystem (e.g., **Memory Arenas**, **Pools**, or **Debug Allocators**).

### First Option: Global Override (Recommended)

To use a custom allocator globally across all `z-libs`, define the `Z_` macros **before** including `zstr.h`.

```c
// my_config.h
#define Z_MALLOC(sz)      my_malloc(sz)
#define Z_CALLOC(n, sz)   my_calloc(n, sz)
#define Z_REALLOC(p, sz)  my_realloc(p, sz)
#define Z_FREE(p)         my_free(p)

#include "zstr.h"
```

### Second Option: Library-Specific Override (Advanced)

If you need a specific allocator just for strings, use the library-specific macros.

```c
// Example: Strings use a specific heap, other z-libs use default.
#define Z_STR_MALLOC(sz)     custom_alloc(sz)
#define Z_STR_REALLOC(p, sz) custom_realloc(p, sz)
#define Z_STR_FREE(p)        custom_free(p)

#include "zstr.h"
```

> **Note for C++:** If you override these macros manually, ensure your `MALLOC` and `REALLOC` macros cast their result to `(char*)` to satisfy C++ strict typing, though `zstr.h` handles this internally for standard headers (just a gentle reminder).

## Notes

### Small String Optimization (SSO)

`zstr` structs are 32 bytes (on 64-bit systems).
* **Long Mode**: 8 bytes pointer, 8 bytes length, 8 bytes capacity, 1 byte flag, 7 bytes padding.
* **Short Mode**: 23 bytes buffer, 1 byte length.

This means strings of length 0-22 are stored entirely within the struct definition. Creating them or modifying them requires zero heap interaction.
