# Cpp Optimization examples

This repository illustrates various C++ concepts related to optimization with simple examples for
educational purposes.

## Examples

| File | Goal |
| --- | --- |
| `restrict_aliasing_demo.cpp` | Demonstrates how `__restrict__` can improve performance by telling the compiler that pointers do not alias. |

## Running An Example

```bash
mkdir -p build
c++ -O3 restrict_aliasing_demo.cpp -o build/restrict_demo
./build/restrict_demo
```
