// Demonstrates how __restrict__ can improve performance by promising the
// compiler that pointers do not alias.
//
// Run:
//   mkdir -p build
//   c++ -O3 restrict_aliasing_demo.cpp -o build/restrict_demo
//   ./build/restrict_demo
//
// Example output:
//   plain:      0.455734 s
//   restrict:   0.0685837 s
//   speedup:    6.64492x
//   checksum:   126.013

#include <chrono>
#include <iostream>
#include <vector>

constexpr int N = 64;
constexpr int ROUNDS = 20'000'000;

__attribute__((noinline))
void scale(float *out, const float *in, const float *factor) {
#pragma clang loop vectorize(disable) interleave(disable)
    for (int i = 0; i < N; ++i) out[i] = in[i] * *factor;
}

__attribute__((noinline))
void scale_restrict(float *__restrict__ out,
                    const float *__restrict__ in,
                    const float *__restrict__ factor) {
#pragma clang loop vectorize(disable) interleave(disable)
    for (int i = 0; i < N; ++i) out[i] = in[i] * *factor;
}

template <class F>
double bench(F f, float *out, const float *in, const float *factor) {
    auto start = std::chrono::steady_clock::now();
    for (int r = 0; r < ROUNDS; ++r) f(out, in, factor);
    auto stop = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(stop - start).count();
}

int main() {
    std::vector<float> in(N), out(N);
    float factor = 1.0001f;

    for (int i = 0; i < N; ++i) in[i] = float(i);

    double plain = bench(scale, out.data(), in.data(), &factor);
    volatile float keep = out[N - 1];

    double restricted = bench(scale_restrict, out.data(), in.data(), &factor);
    keep += out[N - 1];

    std::cout << "plain:      " << plain << " s\n";
    std::cout << "restrict:   " << restricted << " s\n";
    std::cout << "speedup:    " << plain / restricted << "x\n";
    std::cout << "checksum:   " << keep << '\n';
}
