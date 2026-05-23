package bulletbloom.enemy;

import bulletbloom.assets.AssetManager;
import bulletbloom.core.Geometry;
import bulletbloom.player.Player;
import bulletbloom.weapon.Bullet;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.util.List;

/**
 * Hilichurl archer enemy that keeps distance and fires periodic ranged hits.
 */
public final class ArcherEnemy implements Enemy {
    private static final int MAX_HP = 140;
    private static final double SPEED = 0.65;
    private static final double IDEAL_DISTANCE = 430.0;
    private static final int SHOT_COOLDOWN = 150;
    private static final int AIM_TIME = 35;

    private final BufferedImage loadedImage;
    private final BufferedImage unloadedImage;
    private double x;
    private double y;
    private int hp = MAX_HP;
    private int shotCooldown = SHOT_COOLDOWN;
    private boolean alive = true;

    /**
     * Creates an archer enemy at a world position.
     *
     * @param x initial x coordinate
     * @param y initial y coordinate
     */
    public ArcherEnemy(double x, double y) {
        this.x = x;
        this.y = y;
        this.loadedImage = AssetManager.loadImage("/image/enemy/hilichurl_archer/archer_loaded.png");
        this.unloadedImage = AssetManager.loadImage("/image/enemy/hilichurl_archer/archer_unloaded.png");
    }

    @Override
    public void update(Player player, List<Bullet> bullets) {
        if (!alive) {
            return;
        }
        reposition(player);
        updateShot(player);
        applyBulletHits(bullets);
    }

    @Override
    public void draw(Graphics2D graphics) {
        if (!alive) {
            return;
        }
        BufferedImage image = shotCooldown < AIM_TIME ? loadedImage : unloadedImage;
        graphics.drawImage(image, (int) Math.round(x), (int) Math.round(y), null);
        drawHealthBar(graphics);
    }

    @Override
    public boolean isAlive() {
        return alive;
    }

    @Override
    public double getCenterX() {
        return x + getWidth() / 2.0;
    }

    @Override
    public double getCenterY() {
        return y + getHeight() / 2.0;
    }

    private void reposition(Player player) {
        double dx = player.getCenterX() - getCenterX();
        double dy = player.getCenterY() - getCenterY();
        double distance = Math.sqrt(dx * dx + dy * dy);
        if (distance == 0) {
            return;
        }
        double direction = distance < IDEAL_DISTANCE * 0.75 ? -1.0 : 1.0;
        if (Math.abs(distance - IDEAL_DISTANCE) > 60) {
            x += dx / distance * SPEED * direction;
            y += dy / distance * SPEED * direction;
        }
    }

    private void updateShot(Player player) {
        if (shotCooldown > 0) {
            shotCooldown--;
            return;
        }
        player.takeDamage(1);
        shotCooldown = SHOT_COOLDOWN;
    }

    private void applyBulletHits(List<Bullet> bullets) {
        Rectangle bounds = getBounds();
        for (Bullet bullet : bullets) {
            if (!bullet.isActive()) {
                continue;
            }
            if (Geometry.segmentIntersectsRectangle(
                    bullet.getPreviousX(),
                    bullet.getPreviousY(),
                    bullet.getX(),
                    bullet.getY(),
                    bounds)) {
                hp -= bullet.getDamage();
                bullet.deactivate();
                if (hp <= 0) {
                    alive = false;
                    return;
                }
            }
        }
    }

    private void drawHealthBar(Graphics2D graphics) {
        double ratio = Math.max(0.0, hp / (double) MAX_HP);
        graphics.setColor(Color.GREEN);
        graphics.fillRect((int) Math.round(x), (int) Math.round(y) - 10, (int) Math.round(getWidth() * ratio), 5);
        graphics.setColor(Color.BLACK);
        graphics.drawRect((int) Math.round(x), (int) Math.round(y) - 10, getWidth(), 5);
    }

    private Rectangle getBounds() {
        return new Rectangle((int) Math.round(x), (int) Math.round(y), getWidth(), getHeight());
    }

    private int getWidth() {
        return loadedImage.getWidth();
    }

    private int getHeight() {
        return loadedImage.getHeight();
    }
}
