# Shooter Java Rewrite

This repository is being migrated from the original C++/SplashKit shooter assignment into Java in small, reviewable stages.

## Current Stage

Stage 1 creates a JDK-only Java2D/Swing version that can run without Gradle, Maven, or JavaFX installed locally.

Implemented:

- Desktop window at the original `1600x1200` size.
- Main game loop running at the original `120 FPS` target.
- Resource loading from `src/main/resources`.
- Background rendering.
- Player idle and walking animation.
- WASD and arrow-key movement.
- Screen-boundary clamping.

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
5. Stage 5: Wave spawning, boss migration.
6. Stage 6: Menu, pause, shop.
7. Stage 7: Save/load and final debugging pass.

## Notes

- The original C++ source is intentionally kept during migration for behavior comparison.
- New Java classes include Javadoc on public types and methods.
- Stage commits should stay small so each phase can be reviewed before pushing.
