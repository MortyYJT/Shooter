package bulletbloom.enemy;

import java.awt.Rectangle;
import java.util.List;

/**
 * Owns wave number, wave progress, and enemy spawning.
 */
public final class WaveController {
    private final EnemySpawner enemySpawner = new EnemySpawner();
    private int wave = 1;
    private boolean inProgress = true;

    /**
     * Creates a wave controller at wave one.
     */
    public WaveController() {
    }

    /**
     * Updates spawning for the current wave.
     *
     * @param enemies active enemy list
     * @param bounds screen bounds used for spawning
     */
    public void updateSpawning(List<Enemy> enemies, Rectangle bounds) {
        if (!inProgress) {
            return;
        }
        enemySpawner.update(enemies, bounds, wave);
        if (enemySpawner.isWaveComplete(wave, enemies)) {
            inProgress = false;
        }
    }

    /**
     * Starts the next wave after a clear prompt.
     */
    public void startNextWave() {
        wave++;
        inProgress = true;
    }

    /**
     * Resets wave progress for a new run.
     */
    public void reset() {
        wave = 1;
        inProgress = true;
        enemySpawner.reset();
    }

    /**
     * Restores a wave from save data.
     *
     * @param wave restored wave number
     */
    public void restoreWave(int wave) {
        this.wave = Math.max(1, wave);
        this.inProgress = true;
        enemySpawner.reset();
    }

    /**
     * Gets the current wave number.
     *
     * @return wave number
     */
    public int getWave() {
        return wave;
    }

    /**
     * Checks whether the current wave is active.
     *
     * @return {@code true} while enemies are spawning or alive
     */
    public boolean isInProgress() {
        return inProgress;
    }

    /**
     * Gets remaining enemies, including unspawned and active enemies.
     *
     * @param activeEnemyCount number of active enemies
     * @return remaining enemy count
     */
    public int remainingEnemies(int activeEnemyCount) {
        return enemySpawner.getRemainingToSpawn(wave) + activeEnemyCount;
    }
}
