package shooter;

import java.awt.Rectangle;
import java.util.List;
import java.util.Random;

/**
 * Controls timed enemy spawning for the Java gameplay loop.
 */
public final class EnemySpawner {
    private static final int SPAWN_INTERVAL = 60;
    private static final int MAX_ACTIVE_ENEMIES = 8;

    private final Random random = new Random();
    private int spawnTimer;
    private int spawned;
    private int activeWave = -1;

    /**
     * Updates the spawn timer and creates enemies when limits allow.
     *
     * @param enemies active enemy list
     * @param bounds screen bounds used to spawn outside the play area
     * @param wave current wave number
     */
    public void update(List<Enemy> enemies, Rectangle bounds, int wave) {
        if (wave != activeWave) {
            activeWave = wave;
            spawnTimer = 0;
            spawned = 0;
        }

        if (spawned >= maxSpawnedForWave(wave) || enemies.size() >= MAX_ACTIVE_ENEMIES) {
            return;
        }

        spawnTimer++;
        if (spawnTimer < SPAWN_INTERVAL) {
            return;
        }

        spawnTimer = 0;
        enemies.add(spawnSlime(bounds));
        spawned++;
    }

    private Enemy spawnSlime(Rectangle bounds) {
        int side = random.nextInt(4);
        double x;
        double y;
        switch (side) {
            case 0 -> {
                x = random.nextDouble(bounds.getWidth());
                y = -48;
            }
            case 1 -> {
                x = bounds.getMaxX() + 48;
                y = random.nextDouble(bounds.getHeight());
            }
            case 2 -> {
                x = random.nextDouble(bounds.getWidth());
                y = bounds.getMaxY() + 48;
            }
            default -> {
                x = -48;
                y = random.nextDouble(bounds.getHeight());
            }
        }
        return new SlimeEnemy(x, y);
    }

    /**
     * Resets spawn counters for a new run.
     */
    public void reset() {
        spawnTimer = 0;
        spawned = 0;
        activeWave = -1;
    }

    /**
     * Checks whether all enemies for the wave have spawned and been defeated.
     *
     * @param wave current wave number
     * @param enemies active enemy list
     * @return {@code true} when the wave is complete
     */
    public boolean isWaveComplete(int wave, List<Enemy> enemies) {
        return spawned >= maxSpawnedForWave(wave) && enemies.isEmpty();
    }

    /**
     * Gets the number of enemies remaining to spawn in the current wave.
     *
     * @param wave current wave number
     * @return non-negative remaining spawn count
     */
    public int getRemainingToSpawn(int wave) {
        return Math.max(0, maxSpawnedForWave(wave) - spawned);
    }

    /**
     * Gets the spawn target for a wave.
     *
     * @param wave wave number
     * @return number of enemies to spawn
     */
    public int maxSpawnedForWave(int wave) {
        return Math.min(50, 10 + Math.max(0, wave - 1) * 8);
    }

}
