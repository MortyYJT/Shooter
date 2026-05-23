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
    private static final int MAX_SPAWNED = 20;

    private final Random random = new Random();
    private int spawnTimer;
    private int spawned;

    /**
     * Updates the spawn timer and creates enemies when limits allow.
     *
     * @param enemies active enemy list
     * @param bounds screen bounds used to spawn outside the play area
     */
    public void update(List<Enemy> enemies, Rectangle bounds) {
        if (spawned >= MAX_SPAWNED || enemies.size() >= MAX_ACTIVE_ENEMIES) {
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
    }

}
