# Cpp Optimization examples

This repository illustrates various C++ concepts related to optimization with simple examples for
educational purposes.

## Examples

| File | Goal |
| --- | --- |
| `restrict_aliasing_demo.cpp` | Demonstrates how `__restrict__` can improve performance by telling the compiler that pointers do not alias. |
| `SLERP_vS_NLERP.cpp` | Compares SLERP and NLERP quaternion interpolation performance and shows why NLERP is usually faster. |
| `ecs_vs_oop_particles.cpp` | Compares object/AoS particle storage with ECS-style SoA storage for particle integration and temperature updates. |
| `2DParticleSimulation.cpp` | Interactive raylib particle simulation with elastic collisions, gravity, and energy tracking. |

## Running An Example

```bash
mkdir -p build
c++ -O3 restrict_aliasing_demo.cpp -o build/restrict_demo
./build/restrict_demo

c++ -O3 SLERP_vS_NLERP.cpp -o build/slerp_vs_nlerp
./build/slerp_vs_nlerp

c++ -O3 ecs_vs_oop_particles.cpp -o build/ecs_vs_oop_particles
./build/ecs_vs_oop_particles

c++ -O2 2DParticleSimulation.cpp -o build/2DParticleSimulation $(pkg-config --cflags --libs raylib)
./build/2DParticleSimulation
```
