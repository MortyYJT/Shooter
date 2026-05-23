# Bullet Bloom

Bullet Bloom is a Java rewrite of the original C++/SplashKit shooter assignment. The Java version keeps the original pixel-art assets and core top-down shooter loop while moving gameplay into a JDK-only Java2D/Swing runtime.

## Features

- Desktop Java2D/Swing game window at `1600x1200`.
- Resizable game window with aspect-correct viewport scaling.
- Fixed-delay game loop targeting `120 FPS`.
- Classpath-based image resource loading from `src/main/resources`.
- Player movement, facing, idle/walk animation, health, and damage cooldown.
- Mouse aiming and left-click firing.
- Four migrated weapons: pistol, AK-47, shotgun, and AWP.
- Number-key weapon switching.
- Slime enemy waves with chase behavior, health bars, and bullet collision.
- Boss encounter every fifth wave with a boss health bar and low-health sprite feedback.
- Enemy coin drops, animated coin attraction, collection, and money tracking.
- Resource-backed HUD for hearts, money, weapon, wave, remaining enemies, and active coins.
- Title menu, pause menu, wave-clear prompt, game-over screen, and restart flow.
- Intermission shop for healing, max-health upgrades, and weapon unlocks.
- Pause-menu save and title-menu continue via `save/bullet-bloom.properties`.
- Smoke and deterministic logic tests.
- Runnable jar packaging script.

## Controls

| Action | Input |
| --- | --- |
| Start | `Enter` |
| Continue save | `C` on the title screen |
| Move | `WASD` or arrow keys |
| Aim | Mouse |
| Fire | Left mouse button |
| Switch weapons | `1`, `2`, `3`, `4` |
| Pause/resume | `Esc` |
| Shop | `B` while paused or after wave clear |
| Heal in shop | `H` |
| Buy max HP in shop | `M` |
| Save | `S` while paused |
| Restart after game over | `R` |
| Next wave | `Enter` after wave clear |
| Buy/equip weapon in shop | `2`, `3`, `4` |

## Run Locally

Build:

```bash
scripts/build.sh
```

Run:

```bash
scripts/run.sh
```

Test:

```bash
scripts/test.sh
```

Generate Javadoc:

```bash
scripts/javadoc.sh
```

Package and run a jar:

```bash
scripts/package.sh
java -jar out/dist/bullet-bloom.jar
```

## Project Layout

```text
src/main/java/bulletbloom/app      Swing entry point, panel, and loop
src/main/java/bulletbloom/core     Game state, constants, mode, and geometry
src/main/java/bulletbloom/enemy    Enemy contracts, spawning, slime, and boss
src/main/java/bulletbloom/weapon   Bullets and player weapons
src/main/java/bulletbloom/player   Player movement, health, and animation
src/main/java/bulletbloom/shop     Shop state, wallet, and transactions
src/main/java/bulletbloom/ui       HUD, overlay, and shop rendering
src/main/java/bulletbloom/save     Save data and persistence
src/main/java/bulletbloom/assets   Classpath asset loading
src/main/resources             Runtime images and game assets
src/test/java/bulletbloom      Smoke and logic tests
scripts                        Build, run, test, package, and Javadoc helpers
```

## Development Notes

- The Java runtime is intentionally JDK-only; no Gradle, Maven, or external game framework is required.
- Legacy C++ source and duplicate root-level assets have been removed after the Java migration.
- `scripts/test.sh` runs deterministic logic checks first, then an offscreen render smoke test.
- Local Java save data is ignored at `save/bullet-bloom.properties`.
- Generated outputs stay under `out/` and are ignored by Git.
