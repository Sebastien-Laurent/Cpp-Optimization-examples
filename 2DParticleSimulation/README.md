# 2D Particle Simulation

Interactive raylib particle simulation for testing collision detection approaches.

Currently there are two collisions detection apporaches :
- Brute force: checking collisions for all pairs

- Spatial grid: checking collisions for all pairs that are within neighboring cells

Gravity : vertical constant acceleration 
Wall detection: elastic collision 

Intra dynamic collision is computed using Verlet method to conserv energy
Wall collision basically change velocity sign and clamp particle position + some correction when gravity is enabled , to avoid energy loss/ addition.

You can enable / disable :
- collisions and switch collision dectection in real time
- gravity
You can add/remove particles (per batch of 100)
You can pause/resume the simulation
You show hide the particles

Major performance bottlenecks observed:
- Using brute force for collisions : max i can go without FPS drop is around 4k particles
- Drawing circles for particles (now the programm uses a sprite) : max was around 16k particles

Currently, with gravity on, using spatial grid collisions and using sprite texture for particles, the programm can run smoothly until 35k particles.

Remarks:

Collision resolution is very simplistic, it only checks overlap of particles and there is a lot of tunneling going on.


## Build

```bash
make -C 2DParticleSimulation
```

## Run

```bash
make -C 2DParticleSimulation run
```

## Collision Modes

- `Brute Force`: checks every particle pair.
- `Spatial Grid`: uses a uniform-grid broad phase.

