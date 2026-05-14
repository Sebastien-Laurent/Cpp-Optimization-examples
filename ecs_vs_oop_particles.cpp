// Demonstrates why particle systems are often stored in an ECS-style
// Structure of Arrays (SoA) layout instead of a classical object/AoS layout.
//
// The update needs position, velocity, mass, and temperature. In the object
// layout, unrelated fields such as radius, lifetime, color, and id sit in the
// same cache lines. In the ECS-style layout, the integration system walks only
// the arrays it needs.
//
// This is not a full ECS implementation; it focuses on the memory layout idea.
//
// Run:
//   mkdir -p build
//   c++ -O3 ecs_vs_oop_particles.cpp -o build/ecs_vs_oop_particles
//   ./build/ecs_vs_oop_particles
//
// Example output:
//   Particles:  1000000
//   Steps:      200
//   Object/AoS: 316.871 ms
//   ECS/SoA:    109.581 ms
//   Speedup:    2.89165x faster
//   Checksum:   2.31142e+08

#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

constexpr int PARTICLE_COUNT = 1'000'000;
constexpr int STEPS = 200;
constexpr float DT = 0.016f;

struct ParticleObject {
    float x, y, z;
    float vx, vy, vz;
    float mass;
    float radius;
    float lifetime;
    float temperature;
    std::uint32_t color;
    std::uint32_t id;
    float unused0;
    float unused1;
    float unused2;
    float unused3;
};

struct ParticleComponents {
    std::vector<float> x, y, z;
    std::vector<float> vx, vy, vz;
    std::vector<float> mass;
    std::vector<float> radius;
    std::vector<float> lifetime;
    std::vector<float> temperature;
    std::vector<std::uint32_t> color;
    std::vector<std::uint32_t> id;
};

ParticleComponents make_components(int count) {
    ParticleComponents p;
    p.x.resize(count);
    p.y.resize(count);
    p.z.resize(count);
    p.vx.resize(count);
    p.vy.resize(count);
    p.vz.resize(count);
    p.mass.resize(count);
    p.radius.resize(count);
    p.lifetime.resize(count);
    p.temperature.resize(count);
    p.color.resize(count);
    p.id.resize(count);

    for (int i = 0; i < count; ++i) {
        p.x[i] = float(i % 100);
        p.y[i] = float((i / 100) % 100);
        p.z[i] = float(i % 17);
        p.vx[i] = 0.10f;
        p.vy[i] = 0.20f;
        p.vz[i] = 0.30f;
        p.mass[i] = 1.0f;
        p.radius[i] = 0.5f;
        p.lifetime[i] = 10.0f;
        p.temperature[i] = 300.0f;
        p.color[i] = 0xff8844u;
        p.id[i] = std::uint32_t(i);
    }

    return p;
}

std::vector<ParticleObject> make_objects(const ParticleComponents &p) {
    std::vector<ParticleObject> objects(p.x.size());

    for (std::size_t i = 0; i < objects.size(); ++i) {
        objects[i] = {
            p.x[i], p.y[i], p.z[i],
            p.vx[i], p.vy[i], p.vz[i],
            p.mass[i], p.radius[i], p.lifetime[i], p.temperature[i],
            p.color[i], p.id[i],
            0.0f, 0.0f, 0.0f, 0.0f,
        };
    }

    return objects;
}

__attribute__((noinline))
void integrate_objects(std::vector<ParticleObject> &particles) {
    for (ParticleObject &p : particles) {
        p.x += p.vx * DT;
        p.y += p.vy * DT;
        p.z += p.vz * DT;

        float speed2 = p.vx * p.vx + p.vy * p.vy + p.vz * p.vz;
    }
}

__attribute__((noinline))
void integrate_components(ParticleComponents &p) {
    for (std::size_t i = 0; i < p.x.size(); ++i) {
        p.x[i] += p.vx[i] * DT;
        p.y[i] += p.vy[i] * DT;
        p.z[i] += p.vz[i] * DT;

        float speed2 = p.vx[i] * p.vx[i] + p.vy[i] * p.vy[i] + p.vz[i] * p.vz[i];
    }
}

template <class F, class Data>
double bench(F update, Data &data) {
    auto start = std::chrono::steady_clock::now();

    for (int step = 0; step < STEPS; ++step) update(data);

    auto stop = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(stop - start).count();
}

__attribute__((noinline))
float checksum_objects(const std::vector<ParticleObject> &particles) {
    float sum = 0.0f;
    for (const ParticleObject &p : particles) sum += p.x + p.y + p.z + p.temperature;
    return sum;
}

__attribute__((noinline))
float checksum_components(const ParticleComponents &p) {
    float sum = 0.0f;
    for (std::size_t i = 0; i < p.x.size(); ++i) {
        sum += p.x[i] + p.y[i] + p.z[i] + p.temperature[i];
    }
    return sum;
}

int main() {
    ParticleComponents components = make_components(PARTICLE_COUNT);
    std::vector<ParticleObject> objects = make_objects(components);

    integrate_objects(objects);
    integrate_components(components);

    double objectTime = bench(integrate_objects, objects);
    double componentTime = bench(integrate_components, components);

    float checksum = checksum_objects(objects) + checksum_components(components);

    std::cout << "Particles:  " << PARTICLE_COUNT << '\n';
    std::cout << "Steps:      " << STEPS << '\n';
    std::cout << "Object/AoS: " << objectTime << " ms\n";
    std::cout << "ECS/SoA:    " << componentTime << " ms\n";
    std::cout << "Speedup:    " << objectTime / componentTime << "x faster\n";
    std::cout << "Checksum:   " << checksum << '\n';
}
