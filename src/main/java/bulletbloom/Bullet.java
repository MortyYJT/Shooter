package bulletbloom;

import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;

/**
 * Projectile fired by a weapon.
 */
public final class Bullet {
    private final BufferedImage image;
    private final int damage;
    private double x;
    private double y;
    private double previousX;
    private double previousY;
    private final double dx;
    private final double dy;
    private boolean active = true;

    /**
     * Creates a bullet.
     *
     * @param x starting x coordinate
     * @param y starting y coordinate
     * @param angleRadians movement angle in radians
     * @param speed movement speed per update tick
     * @param damage damage dealt on impact
     * @param image bullet sprite
     */
    public Bullet(double x, double y, double angleRadians, double speed, int damage, BufferedImage image) {
        this.x = x;
        this.y = y;
        this.previousX = x;
        this.previousY = y;
        this.dx = Math.cos(angleRadians) * speed;
        this.dy = Math.sin(angleRadians) * speed;
        this.damage = damage;
        this.image = image;
    }

    /**
     * Moves the bullet and deactivates it outside the screen bounds.
     *
     * @param bounds active play area
     */
    public void update(Rectangle bounds) {
        if (!active) {
            return;
        }

        previousX = x;
        previousY = y;
        x += dx;
        y += dy;
        if (!bounds.intersects(x, y, Math.max(1, image.getWidth()), Math.max(1, image.getHeight()))) {
            active = false;
        }
    }

    /**
     * Draws the bullet rotated along its velocity.
     *
     * @param graphics active graphics context
     */
    public void draw(Graphics2D graphics) {
        if (!active) {
            return;
        }

        AffineTransform original = graphics.getTransform();
        graphics.translate(x, y);
        graphics.rotate(Math.atan2(dy, dx), image.getWidth() / 2.0, image.getHeight() / 2.0);
        graphics.drawImage(image, 0, 0, null);
        graphics.setTransform(original);
    }

    /**
     * Checks whether the bullet is still active.
     *
     * @return {@code true} while the bullet should remain in play
     */
    public boolean isActive() {
        return active;
    }

    /**
     * Deactivates the bullet after a collision.
     */
    public void deactivate() {
        active = false;
    }

    /**
     * Gets the bullet damage value.
     *
     * @return damage applied by this bullet
     */
    public int getDamage() {
        return damage;
    }

    /**
     * Gets the current x coordinate.
     *
     * @return current x coordinate
     */
    public double getX() {
        return x;
    }

    /**
     * Gets the current y coordinate.
     *
     * @return current y coordinate
     */
    public double getY() {
        return y;
    }

    /**
     * Gets the previous x coordinate before the latest update.
     *
     * @return previous x coordinate
     */
    public double getPreviousX() {
        return previousX;
    }

    /**
     * Gets the previous y coordinate before the latest update.
     *
     * @return previous y coordinate
     */
    public double getPreviousY() {
        return previousY;
    }
}
