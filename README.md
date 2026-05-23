# Bullet Bloom

This repository is being migrated from the original C++/SplashKit shooter assignment into Java in small, reviewable stages.

## Current Stage

Stage 5 keeps the project JDK-only with Java2D/Swing and now covers the core combat loop: player movement, multiple weapons, slime enemies, coin drops, and HUD rendering.

Implemented:

- Desktop window at the original `1600x1200` size.
- Main game loop running at the original `120 FPS` target.
- Resource loading from `src/main/resources`.
- Background rendering.
- Player idle and walking animation.
- WASD and arrow-key movement.
- Screen-boundary clamping.
- Pistol weapon rendering.
- Mouse aiming.
- Left-click pistol firing.
- Bullet movement, rotation, and offscreen cleanup.
- Slime enemy spawning, chasing, health bars, and bullet damage.
- Basic player contact damage.
- Enemy coin drops, animated coin attraction, collection, and money tracking.
- Resource-backed heart and coin HUD.
- Multiple migrated weapons with number-key switching: pistol, AK-47, shotgun, and AWP.
- Menu, pause, game-over, and restart flow.
- Wave-based slime spawning with clear prompts and next-wave progression.

## Run Locally

Compile:

```bash
javac -d out/classes $(find src/main/java -name '*.java')
```

Copy resources:

```bash
cp -R src/main/resources/* out/classes/
```

Run:

```bash
java -cp out/classes shooter.GameApp
```

Smoke test:

```bash
javac -cp out/classes -d out/test-classes $(find src/test/java -name '*.java')
java -cp out/classes:out/test-classes shooter.GameSmokeTest
```

## Migration Plan

1. Stage 1: Java application shell, loop, input, background, player movement.
2. Stage 2: Weapon model, bullets, firing direction, projectile rendering.
3. Stage 3: Enemy model, slime migration, collision and damage.
4. Stage 4: Coins, health UI, hit feedback.
5. Stage 5: Multi-weapon migration and switching.
6. Stage 6: Menu, pause, and restart flow.
7. Stage 7: Wave spawning.
8. Stage 8: Boss, shop, save/load, and final debugging pass.

## Notes

- The original C++ source is intentionally kept during migration for behavior comparison.
- New Java classes include Javadoc on public types and methods.
- Stage commits should stay small so each phase can be reviewed before pushing.
