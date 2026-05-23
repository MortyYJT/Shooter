package bulletbloom;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.util.List;

/**
 * Boss enemy used for milestone waves in the Java migration.
 */
public final class BossEnemy implements Enemy {
    private static final int MAX_HP = 1600;
    private static final double SPEED = 0.8;

    private final BufferedImage regularImage;
    private final BufferedImage lowHpImage;
    private double x;
    private double y;
    private int hp = MAX_HP;
    private boolean alive = true;

    /**
     * Creates a boss at a world position.
     *
     * @param x initial x coordinate
     * @param y initial y coordinate
     */
    public BossEnemy(double x, double y) {
        this.x = x;
        this.y = y;
        this.regularImage = AssetManager.loadImage("/image/enemy/BOSS/regular.png");
        this.lowHpImage = AssetManager.loadImage("/image/enemy/BOSS/low_hp.png");
    }

    @Override
    public void update(Player player, List<Bullet> bullets) {
        if (!alive) {
            return;
        }

        moveToward(player);
        if (getDamageBounds().intersects(player.getBounds())) {
            player.takeDamage(2);
        }
        applyBulletHits(bullets);
    }

    @Override
    public void draw(Graphics2D graphics) {
        if (!alive) {
            return;
        }

        BufferedImage image = hp < MAX_HP * 0.35 ? lowHpImage : regularImage;
        graphics.drawImage(image, (int) Math.round(x), (int) Math.round(y), null);
        drawHealthBars(graphics);
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

    private void drawHealthBars(Graphics2D graphics) {
        double ratio = Math.max(0.0, hp / (double) MAX_HP);
        int topWidth = 800;
        int topHeight = 16;
        int topX = GameConstants.SCREEN_WIDTH / 2 - topWidth / 2;
        int topY = 60;
        graphics.setColor(hp < MAX_HP * 0.35 ? Color.RED : new Color(122, 64, 180));
        graphics.fillRect(topX, topY, (int) Math.round(topWidth * ratio), topHeight);
        graphics.setColor(Color.BLACK);
        graphics.drawRect(topX, topY, topWidth, topHeight);
    }

    private Rectangle getBounds() {
        return new Rectangle((int) Math.round(x), (int) Math.round(y), getWidth(), getHeight());
    }

    private Rectangle getDamageBounds() {
        return new Rectangle(
                (int) Math.round(x + getWidth() * 0.15),
                (int) Math.round(y + getHeight() * 0.35),
                (int) Math.round(getWidth() * 0.7),
                (int) Math.round(getHeight() * 0.5));
    }

    private int getWidth() {
        return regularImage.getWidth();
    }

    private int getHeight() {
        return regularImage.getHeight();
    }
}
