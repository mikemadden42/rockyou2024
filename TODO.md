# TODO

Findings from the project review on 2026-11-06. Items under "Bugs" were reproduced
unless marked otherwise. Line numbers refer to commit 85cd5ed.

## Bugs in `search.cc`

### High

- [x] **The end of the file is never searched** (`search.cc:36`). `file.read()` returns
  false on a short read, so the loop skips the final partial chunk. Files under 1 MB
  aren't searched at all, and no count is printed for them.
  - A small file with 3 matches printed no count.
  - A 1 MB+ file with 2 matches reported 1.
  - A 2.8 GB test file reported 1,219,281 instead of 1,219,589.
  - Already present before the refactor to modern C++ (`bf16f4c`).
- [x] **`int count` overflows** (`search.cc:35`) on any keyword with more than about
  2.1 billion matches. On 2.3 GB of `a` it printed `-1995440128`, and the UBSan build
  stopped at `search.cc:38`. Change the counters to `std::uint64_t`.
- [x] **An empty keyword hangs forever** (`search.cc:17-19`) on any file of 1 MB or more.
  `find("", pos)` keeps returning `pos`, and `pos` advances by 0.

### Medium

- [x] **A keyword longer than the chunk breaks the seek logic** (`search.cc:42`). At
  exactly 1 MB + 1 byte the file position stops advancing, so it hangs. One length
  check can fix this and the empty keyword.
- [x] **Failures look like success** (`search.cc:36`, `:70`). There's no check for read
  errors, and a directory passes `fs::exists`, so passing one prints "Search complete"
  and no error.

### Low

- [x] Fix the comment at `search.cc:30`, which promises extra buffer space that isn't
  allocated.
- [x] Rename `cpu_time_used` (`search.cc:50`), which is really wall-clock time.

## Performance

- [x] Replace `ifstream` and seeking with unbuffered `std::fread` and `memmove`. This was
  about 30-40% faster and stays portable. The speedup comes from leaving `ifstream`, not
  from dropping the seek: `ifstream` without seeking was no faster than the old code.

Timings for `Hunter02` on a 2.8 GB synthetic file that was already cached:

| Approach | Time | Speed |
|---|---|---|
| Old code (`ifstream` + seek) | 0.85s | about 3,500 MB/s |
| `ifstream` + `memmove`, no seeking | 0.85s | about 3,500 MB/s |
| POSIX `read()` + `memmove` | 0.52s | about 5,800 MB/s |
| **Unbuffered `fread` + `memmove` (current)** | **0.51s** | **about 5,900 MB/s** |
| Boyer-Moore-Horspool | 1.37s | 2,184 MB/s |
| `memmem` | 1.92s | 1,561 MB/s |
| `mmap` + `memmem` | 1.91s | 1,572 MB/s |

Fancier search algorithms and `mmap` were slower than `string_view::find` here, so
they aren't worth pursuing.

The README's 517 seconds works out to about 290 MB/s, far below these speeds, so the
disk is the likely bottleneck. That's an inference: the timing probably came from
different hardware, and the real file wasn't available to test.

## Tests

- [x] Add `ctest` cases for a small file, a match across chunks, a match at the end of
  the file, and an empty keyword.
- [ ] Add a test for count overflow. It needs more than 2.1 billion matches, which took
  about 30 seconds on 2.3 GB of `a`, so it's too slow for a default `ctest` run.

## Build (`CMakeLists.txt`)

- [ ] Remove the `FATAL_ERROR` calls at lines 38 and 46, which stop every MSVC configure
  and leave the MSVC branches as dead code. `add_custom_flags` also passes GCC-style
  flags. (Found by reading the code, not by running it.)
- [ ] Handle MSan on macOS, where the build fails with Apple clang. The other six build
  types pass.
- [x] Delete `add_custom_flags` (lines 74-94). Every sanitizer flag is applied twice, by
  this function and by `CMAKE_CXX_FLAGS_<TYPE>`.
- [x] Stop overwriting `CMAKE_CXX_FLAGS` on line 12, which discards the user's
  `CXXFLAGS`. Use `add_compile_options` instead.
- [x] Set a default build type, since plain `cmake ..` builds without optimization.
- [x] Raise `cmake_minimum_required` to at least 3.13, which `target_link_options`
  requires.

## CI (`.github/workflows/cmake.yml`)

- [x] Make the test step meaningful. `ctest` printed "No tests were found!!!" and exited 0,
  so it always passed. It now fails if no tests are found.
- [x] Build more than Release on Ubuntu. CI now also builds ASAN and UBSAN there, plus
  Release on macOS.
- [x] Update `actions/checkout@v3`.

## README and usability

- [x] Fix `README.md:9`, which says the b3sum is for the `.txt` file. It's for the `.zip`.
- [x] Add a filename argument so the tool can be scripted. Keep reading the keyword from
  stdin, because a password in `argv` ends up in shell history and `ps`.
- [x] Document that matches are substrings: `Hunter02` also counts `xHunter02!`.
- [x] Consider a whole-line `--exact` mode to answer "is this exact password in the list?"
