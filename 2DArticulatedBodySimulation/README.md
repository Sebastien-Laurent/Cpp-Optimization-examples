# 2D Articulated Body Simulation

Interactive raylib simulation for testing articulated-body dynamics algorithms.

The first scene is a double pendulum simulated in generalized coordinates using equations derived from the Lagrangian. The generalized coordinates are:

```text
q = [theta1, theta2]
```

The dynamics are represented in the standard form:

```text
M(q) q'' + h(q, q') = 0
```

The app uses a fixed timestep and tracks energy drift so integrators can be compared directly. The implicit integrators build a nonlinear timestep residual and solve it with Newton-Raphson using a finite-difference Jacobian.

## Current Integrators

- `Explicit Euler`
- `Semi-Implicit Euler`
- `RK4`
- `Implicit Euler`
- `Implicit Midpoint`

## Controls

- `Space`: pause/resume
- `R`: reset pendulum
- UI buttons: switch integrator, adjust Newton iterations for implicit integrators, toggle trail

## Build

```bash
make -C 2DArticulatedBodySimulation
```

## Run

```bash
make -C 2DArticulatedBodySimulation run
```
