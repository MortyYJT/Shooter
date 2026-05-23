package shooter;

import java.awt.Graphics2D;
import java.util.List;

/**
 * Common behavior for active enemies.
 */
public interface Enemy {
    /**
     * Updates movement, player contact, and bullet collision.
     *
     * @param player current player
     * @param bullets active bullets that can damage this enemy
     */
    void update(Player player, List<Bullet> bullets);

    /**
     * Draws the enemy.
     *
     * @param graphics active graphics context
     */
    void draw(Graphics2D graphics);

    /**
     * Checks whether this enemy is still alive.
     *
     * @return {@code true} while the enemy should stay in play
     */
    boolean isAlive();
}
