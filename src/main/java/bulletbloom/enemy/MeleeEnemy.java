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
 * Hilichurl melee enemy that closes distance and attacks on contact.
 */
public final class MeleeEnemy implements Enemy {
    private static final int MAX_HP = 180;
    private static final double SPEED = 0.9;
    private static final int ATTACK_COOLDOWN = 80;

    private final BufferedImage idleImage;
    private final BufferedImage[] attackFrames = new BufferedImage[3];
    private double x;
    private double y;
    private int hp = MAX_HP;
    private int attackCooldown;
    private boolean alive = true;

    /**
     * Creates a melee enemy at a world position.
     *
     * @param x initial x coordinate
     * @param y initial y coordinate
     */
    public MeleeEnemy(double x, double y) {
        this.x = x;
        this.y = y;
        this.idleImage = AssetManager.loadImage("/image/enemy/hilichurl/melee_idle.png");
        for (int index = 0; index < attackFrames.length; index++) {
            attackFrames[index] = AssetManager.loadImage("/image/enemy/hilichurl/melee_attack_" + index + ".png");
        }
    }

    @Override
    public void update(Player player, List<Bullet> bullets) {
        if (!alive) {
            return;
        }
        if (attackCooldown > 0) {
            attackCooldown--;
        }
        moveToward(player);
        attackPlayer(player);
        applyBulletHits(bullets);
    }

    @Override
    public void draw(Graphics2D graphics) {
        if (!alive) {
            return;
        }
        BufferedImage image = attackCooldown > ATTACK_COOLDOWN - 30
                ? attackFrames[(ATTACK_COOLDOWN - attackCooldown) / 10 % attackFrames.length]
                : idleImage;
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

    private void moveToward(Player player) {
        double dx = player.getCenterX() - getCenterX();
        double dy = player.getCenterY() - getCenterY();
        double length = Math.sqrt(dx * dx + dy * dy);
        if (length > 0) {
            x += dx / length * SPEED;
            y += dy / length * SPEED;
        }
    }

    private void attackPlayer(Player player) {
        if (attackCooldown == 0 && getDamageBounds().intersects(player.getBounds())) {
            player.takeDamage(2);
            attackCooldown = ATTACK_COOLDOWN;
        }
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

    private Rectangle getDamageBounds() {
        return new Rectangle((int) Math.round(x), (int) Math.round(y + getHeight() * 0.35), getWidth(), getHeight() / 2);
    }

    private int getWidth() {
        return idleImage.getWidth();
    }

    private int getHeight() {
        return idleImage.getHeight();
    }
}
