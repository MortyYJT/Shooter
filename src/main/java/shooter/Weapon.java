package shooter;

import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.util.List;

/**
 * Common behavior for player weapons.
 */
public interface Weapon {
    /**
     * Updates firing state and active projectiles.
     *
     * @param player player using the weapon
     * @param input current input state
     * @param bounds active play area
     */
    void update(Player player, InputManager input, Rectangle bounds);

    /**
     * Draws the weapon and projectiles.
     *
     * @param graphics active graphics context
     * @param player player using the weapon
     */
    void draw(Graphics2D graphics, Player player);

    /**
     * Gets active bullets owned by this weapon.
     *
     * @return bullet list
     */
    List<Bullet> getBullets();

    /**
     * Gets the display name for the weapon.
     *
     * @return weapon name
     */
    String getName();
}
