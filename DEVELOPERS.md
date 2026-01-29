# GS1 Barcode Syntax Dictionary and Syntax Tests

This document is for developers contributing to this project, not users of the
project. It is authoritative for both human contributors and automated tools.


## Background

This repository contains two related components:

The **GS1 Barcode Syntax Dictionary** ("Syntax Dictionary") is a text file
(`gs1-syntax-dictionary.txt`) that is both human-readable and machine-readable,
describing each currently assigned GS1 Application Identifier in terms of its
constituent components for the purpose of input validation.

The **GS1 Barcode Syntax Tests** ("Linters") are C reference implementations of
validation routines that are referred to by the AI entries within the Syntax
Dictionary. They validate:

- Character sets (CSET 39, 64, 82, numeric)
- Check digits and checksums
- Date and time formats (YYMMDD, YYYYMMDD, HHMI, etc.)
- International standards (ISO 3166, ISO 4217, ISO 5218, IBAN)
- GS1-specific formats (coupon codes, GCP positions, media types, package types)

The Linters are the authoritative source for validation implementations. The
sibling project GS1 Syntax Engine vendors these linters and provides a
framework with language bindings.


### Syntax Dictionary Contributions

The Syntax Dictionary file format is documented in the file's header.

Entries describe AI structure in terms of how to validate the data, not as a
semantic decomposition into meaningful components. Linter references are
validation function references and do not confer any additional semantics to
their components.

Importantly:

- Maintain entries in lexical order of their AI
- Changes to linter references must be coordinated with corresponding updates to the C linter code
- Prefer combined `req=` expressions (e.g., `req=A+C,B+C`) over multiple `req=` attributes for more informative error messages
- Prefer optional components over variable-length when the shorter form is valid (e.g., `N19 [N1]` rather than `N18 N..2` when 19 digits alone is valid input)
- Do not add the `?` flag to an AI unless the GS1 Digital Link specification permits it as a data attribute

### C Code Contribution Rules and Constraints

- Create atomic, logical commits that complete one task
- Match existing code style and idioms
- All C code must compile cleanly with `-Wall -Wextra -Wconversion -Werror -pedantic`
- C code must compile on MSVC, GCC, and Clang (including Apple's Clang variant)
- Use `const` liberally - both for pointer targets and the pointers themselves: `const char* const data`
- Ensure unit tests cover new functionality and error conditions
- Use Doxygen-style comments for all linter functions (`@param`, `@return`, `@note`)
- Document code that works around compiler bugs or satiates static analysers with a comment explaining why

## LLM Rules

- Do not push commits or rewrite history
- If a file is updated then its copyright date should be bumped to the current year
- Backup work before running irreversible commands
- Plan complex tasks; interview the user regarding significant design choices
- Always search for and follow pre-existing patterns before writing code


## C Programming Goals

The library is suitable for embedded, desktop, and server environments. Code
should be portable and avoid platform-specific dependencies.

**Avoid deeply nested code** by exiting blocks early with `return`, `continue`,
`break`.

### Portability Considerations

- Avoid platform-specific headers and functions
- Use `DIAG_{PUSH,POP,DISABLE_*}` macros to suppress warnings in specific code sections while maintaining `-Werror` builds
- Use `size_t` for iterating unbounded memory, otherwise native-width `int` if sufficient; avoid smaller types that reduce performance

### Performance Patterns

The choice of lookup strategy depends on the density of valid values within the
search domain, not the data type.

**Bitfield lookups for densely populated domains:**

```c
// Define allowed characters as a bitfield (128 bits for ASCII)
static const uint8_t cset[16] = { /* bits for each allowed char */ };

// Check character in O(1)
if (!GS1_LINTER_BITFIELD_LOOKUP(cset, c))
    GS1_LINTER_RETURN_ERROR(GS1_LINTER_INVALID_CSET_CHARACTER, pos, 1);
```

**Binary search for sparsely populated domains:**

```c
static const char* const valid_codes[] = { "AA", "EX", ..., "ZZ" /* sorted */ };

GS1_LINTER_BINARY_SEARCH(data, valid_codes, SIZEOF_ARRAY(valid_codes));
```

**Direct checks for small domains:**

```c
// For very small sets, explicit comparisons are simplest
if (data[0] != 'Y' && data[0] != 'N' && data[0] != 'U')
    GS1_LINTER_RETURN_ERROR(GS1_LINTER_INVALID_VALUE, 0, 1);
```

**Branch hints for error paths:**

```c
if (GS1_LINTER_UNLIKELY(data_len < 6))
    GS1_LINTER_RETURN_ERROR(GS1_LINTER_TOO_SHORT, 0, data_len);
```

### Assertions

Use `assert()` for preconditions:

```c
gs1_lint_err_t gs1_lint_example(const char* const data, ...)
{
    assert(data);  // Caller must provide valid pointer
    // ...
}
```

Assertions are extremely valuable for fuzzing - they catch invariant violations
that might otherwise cause silent corruption.

### Custom Lookup Hooks

Linters can support custom lookup implementations via preprocessor hooks:

```c
#ifdef GS1_LINTER_CUSTOM_ISO4217_LOOKUP_H
#include GS1_LINTER_CUSTOM_ISO4217_LOOKUP_H
#else
// Default implementation
static const char* const iso4217[] = { "AED", "AFN", ... };
#endif
```

This allows users to provide their own data sources (e.g., database lookups)
without modifying the library code.


## Code Structure

### Key Files

| File                          | Purpose                                               |
|-------------------------------|-------------------------------------------------------|
| `gs1-syntax-dictionary.txt`   | The Syntax Dictionary data file                       |
| `gs1syntaxdictionary.h`       | Public API with error codes and function declarations |
| `gs1syntaxdictionary-utils.h` | Utility macros used by linter implementations         |
| `gs1syntaxdictionary.c`       | Maps linter names to functions; error string lookup   |
| `lint_*.c`                    | Individual linter implementations with embedded tests |
| `unittest.h`                  | Wrappers around acutest for consistent test macros    |


### Naming Conventions

| Scope            | Prefix                       | Example                          |
|------------------|------------------------------|----------------------------------|
| Linter functions | `gs1_lint_`                  | `gs1_lint_csum()`                |
| Error codes      | `GS1_LINTER_`                | `GS1_LINTER_ILLEGAL_DAY`         |
| Utility macros   | `GS1_LINTER_`                | `GS1_LINTER_RETURN_OK`           |
| Types            | `gs1_lint_` or `gs1_linter_` | `gs1_lint_err_t`, `gs1_linter_t` |


### Linter File Structure

Each `lint_*.c` file follows this pattern:

```c
/*
 * GS1 Barcode Syntax Dictionary. Copyright (c) 2022-2026 GS1 AISBL.
 * [Apache 2.0 license header]
 */

/**
 * @file lint_example.c
 *
 * @brief The `example` linter ensures that...
 */

#include <assert.h>

#include "gs1syntaxdictionary.h"
#include "gs1syntaxdictionary-utils.h"

/**
 * Used to validate...
 *
 * @param [in] data         Pointer to the data to be linted. Must not be `NULL`.
 * @param [in] data_len     Length of the data to be linted.
 * @param [out] err_pos     Start position of bad data (if error).
 * @param [out] err_len     Length of bad data (if error).
 *
 * @return #GS1_LINTER_OK   if okay.
 * @return #GS1_LINTER_...  if specific error.
 */
GS1_SYNTAX_DICTIONARY_API gs1_lint_err_t gs1_lint_example(
    const char* const data, size_t data_len,
    size_t* const err_pos, size_t* const err_len)
{
    assert(data);

    // Exit with a specific gs1_lint_err_t error code on error
    if (GS1_LINTER_UNLIKELY(data_len < 4))
        GS1_LINTER_RETURN_ERROR(GS1_LINTER_TOO_SHORT, 0, data_len);

    ...

    GS1_LINTER_RETURN_OK;  // Exits with GS1_LINTER_OK on success
}

#ifdef UNIT_TESTS

#include "unittest.h"  // Always wrapped in ifdef UNIT_TESTS

void test_lint_example(void)
{
    UNIT_TEST_PASS(gs1_lint_example, "valid_input");
    UNIT_TEST_FAIL(gs1_lint_example, "bad", GS1_LINTER_TOO_SHORT, "*bad*");
}

#endif  /* UNIT_TESTS */
```


### Symbol Visibility

The library is compiled with `-fvisibility=hidden`. Public functions use:

```c
GS1_SYNTAX_DICTIONARY_API gs1_lint_err_t gs1_lint_example(...);
```


## Build

### Linux/macOS

```bash
cd src

# Build both static and shared libraries (default)
make -j $(nproc)

# Build specific targets
make -j $(nproc) lib         # Both libraries
make -j $(nproc) libstatic   # Static library only (.a)
make -j $(nproc) libshared   # Shared library only (.so)
```

Note: On macOS use `sysctl -n hw.ncpu` instead of `$(nproc)`.

### Windows

```bash
msbuild /p:Configuration=release gs1syntaxdictionary.sln
```


## Testing

### Unit Tests

```bash
cd src

# Run all tests
make -j $(nproc) test

# Run with sanitizers (recommended)
make -j $(nproc) test SANITIZE=yes

# Run including slow tests
make -j $(nproc) test SLOW_TESTS=yes

# Run specific test suite (regex match on test name)
make test TEST="test_lint_csum"
```

### Fuzzing

```bash
cd src
make -j $(nproc) fuzzer

# Then run the printed commands, e.g.:
ASAN_OPTIONS="symbolize=1 detect_leaks=1" ./build/gs1syntaxdictionary-fuzzer-lint_csum -jobs=$(nproc) -workers=$(nproc) corpus-lint_csum -max_len=125
```


## Maintenance Tasks

### Version Numbering

This project uses **date-based versioning** in YYYY-MM-DD format (e.g.,
`2026-01-27`). Each release corresponds to a specific date when changes were
finalised.

### Updating CHANGES

When documenting changes, reference GS1 Change Control Numbers (GSCNs) where
applicable. GSCNs identify approved changes to GS1 standards and provide
traceability between library updates and specification changes.

Example entry:
```
- Updated AI (8030) per GSCN 24-123
```

### Update Copyright Year

```bash
make -C src copyright
```

### Generate Documentation

```bash
make -C src docs
```

This generates HTML documentation in the `docs/` directory.

### Publicly Documented Implementation

The linters serve as **reference implementations** with publicly documented
source code. The Doxygen configuration uses:

- `INLINE_SOURCES = YES`     - Full source code is embedded in the HTML documentation
- `FILE_PATTERNS = lint_*.c` - Only linter files are documented (not infrastructure)
- `STRIP_CODE_COMMENTS = NO` - All comments are preserved in the source listing

This enables developers to:

- **Transliterate** linters to other programming languages
- **Cross-check** transliterated implementations against the reference
- **Port unit tests** to verify correctness of new implementations

The published documentation is available at:
https://ref.gs1.org/tools/gs1-barcode-syntax-resource/syntax-tests/implementation/

### Adding a New Linter

To add a new Linter:

1. Create `src/lint_<name>.c` following the file structure pattern above
2. Add the error codes to `gs1syntaxdictionary.h` (in the `gs1_lint_err_t` enum)
3. Add the corresponding error messages to `gs1_lint_err_str` in `gs1syntaxdictionary.c`
4. Add the function declaration to `gs1syntaxdictionary.h`
5. Add the name-to-function mapping in `gs1syntaxdictionary.c`
6. Add the test function declaration to `unittest.h` (guarded by `#ifdef UNIT_TESTS`)
7. Add the test function call in `gs1syntaxdictionary-test.c`
8. Run `make test SANITIZE=yes` to verify
9. Run `make docs` to regenerate the documentation

The order of error messages in `gs1_lint_err_str` must match the order of error
codes in `gs1_lint_err_t`. A unit test verifies that the array size matches the
enum size.

### Deprecating a Linter

When a linter is deprecated:

1. Move the implementation to `lint__stubs.c`
2. Mark with the `DEPRECATED` macro
3. Keep the function signature to maintain ABI compatibility
4. Document the deprecation in `CHANGES`


## CI/CD

GitHub Actions workflow (`.github/workflows/gs1syntaxdictionary.yml`) runs:

- Linux CI with gcc and clang (with sanitizers)
- Linux CI with slow tests enabled
- Windows CI with MSVC
- macOS CI with clang (noleak sanitizers)
- Static analysis (scan-build, cppcheck, IWYU)
- CodeQL security analysis

Releases are created when tags are pushed.


### Testing in gs1-syntax-engine

When the Syntax Dictionary or linters are updated, they should also be tested
in the gs1-syntax-engine project to verify integration:

```bash
cd [GS1 Syntax Engine]/src/c-lib
make syncsyntaxdict
make test SANITIZE=yes
make fuzzer  # Run fuzzers to check for edge cases
```

This ensures changes work correctly within the full framework context and
catches edge cases that unit tests may miss.

