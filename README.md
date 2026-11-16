
---

# unique-id

A tiny, dependency-free, thread-safe unique ID generator for C++98 on Linux.

Generates a 32-character hexadecimal ID with no separators, no external libraries, and no reliance on `libuuid`, Boost, or any other third party. Just the C++98 standard library and `/dev/urandom`.

## Features

- **C++98-only** — compiles cleanly with `-std=c++98 -pedantic-errors`.
- **No external libraries** — no Boost, no `libuuid`, no `pthread`, no `util-linux`.
- **Thread-safe** — the internal counter is incremented atomically with a GCC/Clang builtin.
- **Strictly unique per process** — the counter is mixed through a bijective 64-bit function and XORed into the random bytes. Two calls in the same running process can never return the same ID.
- **Probabilistically unique across processes** — 128 bits of `/dev/urandom` make cross-process / cross-run collisions astronomically unlikely (~2⁻¹²²).
- **Opaque output** — the counter is not visible. Consecutive IDs look uncorrelated because the sequential value is mixed with a bijection and hidden under random bytes.

## Output

Exactly 32 hex characters, lowercase, no dashes, no `0x` prefix:

```
f0a5006619c214e886b3120cda819f2c
3b91e77a04cd5f1288f10a2c6dbb4e05
c4f2a1b90e3d7a6c51b8e40d2fa9c731
```

## Requirements

- **Linux** (needs `/dev/urandom`).
- **64-bit `unsigned long`** (LP64). True on every 64-bit Linux distro. A compile-time check (`assert_u64_is_64bit`) fails the build on platforms where `unsigned long` is 32-bit.
- **GCC or Clang** (needs the `__sync_add_and_fetch` builtin).
- **C++98** or later.

## Usage

Copy the code into your project, then:

```cpp
#include <iostream>
#include <string>

// ... the make_unique_id() definition goes somewhere in your project ...

int main()
{
    std::string id = unique_id::make_unique_id();
    std::cout << id << "\n";   // e.g. f0a5006619c214e886b3120cda819f2c
    return 0;
}
```

Compile:

```bash
g++ -std=c++98 -pedantic-errors your_file.cpp -o your_program
```

No extra linker flags. No `-luuid`. No `-lboost_*`. No `-lpthread`.

## How it works

`make_unique_id()` combines two ingredients:

1. **128 random bits** read from `/dev/urandom`.
2. **A 64-bit counter**, incremented atomically on every call.

The counter is first passed through `mix64()`, a bijection on 64-bit integers built from three invertible steps:

```text
x += 0x9E3779B97F4A7C15
x  = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9
x  = (x ^ (x >> 27)) * 0x94D049BB133111EB
x  =  x ^ (x >> 31)
```

Because every step is invertible, distinct inputs map to distinct outputs: the mixer cannot make two different counter values collide. The mixed counter is then XORed into bytes 8–15 of the 16 random bytes, and the whole 16-byte buffer is printed as 32 hex characters.

Two properties follow directly from this construction:

- **In-process uniqueness is guaranteed**, not just probable. If `n₁ ≠ n₂`, then `mix64(n₁) ≠ mix64(n₂)`, so the two XOR patterns differ, so the two output buffers differ — regardless of what `/dev/urandom` returned. Even a broken `/dev/urandom` that returned all zeros on every call would still produce strictly distinct IDs.
- **Cross-process uniqueness is probabilistic.** Different runs draw fresh random bytes and start the counter at `1` again, so uniqueness between processes relies on the random half.

## Limitations

- **Linux-only.** Uses `/dev/urandom`. Not portable to Windows or macOS without changes.
- **64-bit `unsigned long` required.** On 32-bit platforms the compile-time check fails; a portable implementation would need two 32-bit halves or a non-standard 64-bit type.
- **GCC/Clang builtin.** `__sync_add_and_fetch` is not standard C++; MSVC and other compilers reject it. On single-threaded builds it can be replaced with `++cnt`.
- **Not cryptographic.** `mix64` is public and invertible. If an attacker ever obtains the underlying random bytes, they can recover the counter. For cryptographic unpredictability, XOR the mixed counter with a per-process salt drawn from `/dev/urandom` at startup.
- **Not globally ordered.** Two IDs returned in different processes are not sortable or comparable in any meaningful way — they are just distinct.

---

## 📜 License

This project is licensed under the GNU General Public License v3.0 — see the [LICENSE](LICENSE) file for details.

---
