# 2D Articulated Body Simulation

Interactive raylib simulation for testing articulated-body dynamics algorithms.

The current scene is a 10-link pendulum chain represented as an articulated tree. Each revolute link contributes one generalized coordinate:

```text
q = [joint_angle_0, joint_angle_1, ...]
```

The runtime dynamics evaluator builds the Lagrangian system numerically for the current tree:

```text
M(q) q'' + C(q, q') = Q(q)
```

The mass matrix and gravity generalized forces are computed from analytic tree endpoint Jacobians. Velocity terms are computed from analytic derivatives of the mass matrix using tree endpoint Hessians. The app uses a fixed timestep and tracks energy drift so integrators can be compared directly. The implicit integrators build a nonlinear timestep residual and solve it with Newton-Raphson using a finite-difference Jacobian.

The default 10-link scene uses RK4 at a 240 Hz simulation timestep. Implicit methods are still available from the UI, but their Newton solve is more expensive for this larger system.

## Body Presets

- `Simple Pendulum`
- `Double Pendulum`
- `10-Link Chain`
- `Y Pendulum`

## Current Integrators

- `Explicit Euler`
- `Semi-Implicit Euler`
- `RK4`
- `Implicit Euler`
- `Implicit Midpoint`

## RK4 Tradeoffs

RK4 is fast in the current app because it is explicit: each timestep performs four dynamics evaluations and no nonlinear solve. It is a good default for medium-size examples such as the 10-link chain.

Its drawbacks:

- It is not symplectic, so it does not preserve mechanical energy exactly over long simulations.
- It can still drift in energy, especially with chaotic systems, large timesteps, or stiff dynamics.
- It has no built-in damping or stabilization.
- It does not let you take arbitrarily large timesteps; stability still depends on the fastest motion in the system.
- It scales linearly with the number of dynamics evaluations per step, but each dynamics evaluation currently builds dense matrices, so large trees will still become expensive.

RK4 can scale to larger trees for experimentation, but not indefinitely with the current dense Lagrangian implementation. For large articulated trees, recursive dynamics or sparse structured solvers would scale better.

## Implicit Scheme Optimizations

The implicit integrators are currently expensive because Newton-Raphson builds its Jacobian with finite differences. For a tree with `n` generalized coordinates, the implicit unknown has size `2n`, so every Newton iteration needs about `2n + 1` residual evaluations.

Possible optimizations:

- Use an analytic Newton Jacobian instead of finite differences.
- Use automatic differentiation to compute the Jacobian.
- Use quasi-Newton/Broyden updates to avoid rebuilding the full Jacobian every iteration.
- Reduce Newton iterations adaptively based on residual convergence.
- Reuse/factorize matrices when the state changes slowly.
- Exploit sparsity or banded structure in the tree instead of dense matrix solves.
- Replace dense mass-matrix construction with recursive articulated-body algorithms.
- Use implicit methods mainly for smaller/stiffer benchmark cases, and explicit/symplectic methods for large interactive trees.

## Source Layout

- `articulated_tree.*`: tree state, chain factory, forward kinematics, endpoint Jacobians/Hessians.
- `tree_dynamics.*`: mass matrix, gravity forces, velocity forces, forward dynamics.
- `integrators.*`: explicit, RK4, and implicit Newton timestepping.
- `linear_algebra.*`: dense vector/matrix helpers and linear solve.
- `simulation.*`, `ui.*`, `metrics.*`: app state, controls, drawing, and energy reporting.

## Controls

- `Space`: pause/resume
- `R`: reset tree
- UI buttons: switch body preset, switch integrator, adjust initial amplitude, adjust Newton iterations for implicit integrators, toggle trail

## Build

```bash
make -C 2DArticulatedBodySimulation
```

## Run

```bash
make -C 2DArticulatedBodySimulation run
```
