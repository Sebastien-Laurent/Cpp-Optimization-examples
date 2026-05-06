#include <chrono>
#include <cmath>
#include <iostream>

// Demonstrates that NLERP is usually faster than SLERP for quaternion
// interpolation because it avoids trigonometric functions.
//
// Run:
//   mkdir -p build
//   c++ -O3 SLERP_vS_NLERP.cpp -o build/slerp_vs_nlerp
//   ./build/slerp_vs_nlerp
//
// Example output:
//   Results for 1000000 iterations:
//   SLERP:   21.4592 ms
//   NLERP:   4.064 ms
//   Speedup: 5.28032x faster
//   Checksum: 2.54287e+06

struct Quaternion {
    float x, y, z, w;
};

constexpr int ITERATIONS = 1'000'000;
constexpr float INV_SQRT_2 = 0.70710678118f;

float dot(Quaternion a, Quaternion b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

Quaternion normalize(Quaternion q) {
    float mag = std::sqrt(dot(q, q));
    return {q.x / mag, q.y / mag, q.z / mag, q.w / mag};
}

__attribute__((noinline))
Quaternion nlerp(Quaternion q0, Quaternion q1, float t) {
    float dot = q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;
    if (dot < 0.0f) q1 = {-q1.x, -q1.y, -q1.z, -q1.w};

    return normalize({
        q0.x + t * (q1.x - q0.x),
        q0.y + t * (q1.y - q0.y),
        q0.z + t * (q1.z - q0.z),
        q0.w + t * (q1.w - q0.w),
    });
}

__attribute__((noinline))
Quaternion slerp(Quaternion q0, Quaternion q1, float t) {
    float d = dot(q0, q1);

    if (d < 0.0f) {
        q1 = {-q1.x, -q1.y, -q1.z, -q1.w};
        d = -d;
    }

    if (d > 0.9995f) return nlerp(q0, q1, t);
    if (d > 1.0f) d = 1.0f;

    float theta = std::acos(d);
    float sinTheta = std::sin(theta);
    float ratioA = std::sin((1.0f - t) * theta) / sinTheta;
    float ratioB = std::sin(t * theta) / sinTheta;

    return {
        q0.x * ratioA + q1.x * ratioB,
        q0.y * ratioA + q1.y * ratioB,
        q0.z * ratioA + q1.z * ratioB,
        q0.w * ratioA + q1.w * ratioB,
    };
}

template <class F>
double bench(F f, Quaternion qStart, Quaternion qEnd, float &checksum) {
    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < ITERATIONS; ++i) {
        float t = float(i & 1023) / 1023.0f;
        Quaternion result = f(qStart, qEnd, t);
        checksum += result.x + result.y + result.z + result.w;
    }

    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

int main() {
    Quaternion qStart = {0, 0, 0, 1};
    Quaternion qEnd = {INV_SQRT_2, 0, 0, INV_SQRT_2};
    float checksum = 0.0f;

    double slerpTime = bench(slerp, qStart, qEnd, checksum);
    double nlerpTime = bench(nlerp, qStart, qEnd, checksum);

    std::cout << "Results for " << ITERATIONS << " iterations:\n";
    std::cout << "SLERP:   " << slerpTime << " ms\n";
    std::cout << "NLERP:   " << nlerpTime << " ms\n";
    std::cout << "Speedup: " << slerpTime / nlerpTime << "x faster\n";
    std::cout << "Checksum: " << checksum << '\n';
}
