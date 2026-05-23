package bulletbloom;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.util.List;
import java.util.Random;

/**
 * Slime enemy that chases the player and takes bullet damage.
 */
public final class SlimeEnemy implements Enemy {
    private static final String[] COLORS = {"yellow", "purple", "red", "blue"};
    private static final Random RANDOM = new Random();
    private static final int MAX_HP = 100;
    private static final int FRAME_INTERVAL = 30;
    private static final double SPEED = 0.6;
    private static final double SLOWED_SPEED_FACTOR = 0.5;

    private final BufferedImage[] rightFrames = new BufferedImage[2];
    private final BufferedImage[] leftFrames = new BufferedImage[2];
    private double x;
    private double y;
    private int hp = MAX_HP;
    private boolean alive = true;
    private boolean facingLeft;
    private int frame;
    private int frameTimer;
    private int slowTimer;

    /**
     * Creates a slime at a world position.
     *
     * @param x initial x coordinate
     * @param y initial y coordinate
     */
    public SlimeEnemy(double x, double y) {
        this.x = x;
        this.y = y;
        loadAssets();
    }

    @Override
    public void update(Player player, List<Bullet> bullets) {
        if (!alive) {
            return;
        }

        updateAnimation();
        moveToward(player);
        damagePlayerOnContact(player);
        applyBulletHits(bullets);
    }

    @Override
    public void draw(Graphics2D graphics) {
        if (!alive) {
            return;
        }

        BufferedImage image = facingLeft ? leftFrames[frame] : rightFrames[frame];
        graphics.drawImage(image, (int) Math.round(x), (int) Math.round(y), null);
        drawHealthBar(graphics);
    }

    @Override
    public boolean isAlive() {
        return alive;
    }

    private void loadAssets() {
        String color = COLORS[RANDOM.nextInt(COLORS.length)];
        String prefix = "/image/enemy/slime/slime_" + color;
        rightFrames[0] = AssetManager.loadImage(prefix + "_0.png");
        rightFrames[1] = AssetManager.loadImage(prefix + "_1.png");
        leftFrames[0] = AssetManager.loadImage(prefix + "_0_1.png");
        leftFrames[1] = AssetManager.loadImage(prefix + "_1_1.png");
    }

    private void updateAnimation() {
        frameTimer++;
        if (frameTimer >= FRAME_INTERVAL) {
            frameTimer = 0;
            frame = (frame + 1) % rightFrames.length;
        }
        if (slowTimer > 0) {
            slowTimer--;
        }
    }

    private void moveToward(Player player) {
        double dx = player.getCenterX() - getCenterX();
        double dy = player.getCenterY() - getCenterY();
        double length = Math.sqrt(dx * dx + dy * dy);
        if (length == 0) {
            return;
        }

        double speed = slowTimer > 0 ? SPEED * SLOWED_SPEED_FACTOR : SPEED;
        x += dx / length * speed;
        y += dy / length * speed;
        facingLeft = dx > 0;
    }

    private void damagePlayerOnContact(Player player) {
        if (getDamageBounds().intersects(player.getBounds())) {
            player.takeDamage(1);
        }
    }

    private void applyBulletHits(List<Bullet> bullets) {
        Rectangle bounds = getBounds();
        for (Bullet bullet : bullets) {
            if (!bullet.isActive()) {
                continue;
            }

            boolean hit = Geometry.segmentIntersectsRectangle(
                    bullet.getPreviousX(),
                    bullet.getPreviousY(),
                    bullet.getX(),
                    bullet.getY(),
                    bounds);
            if (hit) {
                hp -= bullet.getDamage();
                slowTimer = 60;
                bullet.deactivate();
                if (hp <= 0) {
                    alive = false;
                    return;
                }
            }
        }
    }

    private void drawHealthBar(Graphics2D graphics) {
        int barWidth = getWidth();
        int barHeight = 5;
        int barX = (int) Math.round(x);
        int barY = (int) Math.round(y) - 10;
        double ratio = Math.max(0.0, hp / (double) MAX_HP);
        graphics.setColor(Color.GREEN);
        graphics.fillRect(barX, barY, (int) Math.round(barWidth * ratio), barHeight);
        graphics.setColor(Color.BLACK);
        graphics.drawRect(barX, barY, barWidth, barHeight);
    }

    private Rectangle getBounds() {
        return new Rectangle((int) Math.round(x), (int) Math.round(y), getWidth(), getHeight());
    }

    private Rectangle getDamageBounds() {
        int damageY = (int) Math.round(y + getHeight() * 0.55);
        int damageHeight = Math.max(1, (int) Math.round(getHeight() * 0.45));
        return new Rectangle((int) Math.round(x), damageY, getWidth(), damageHeight);
    }

    @Override
    public double getCenterX() {
        return x + getWidth() / 2.0;
    }

    @Override
    public double getCenterY() {
        return y + getHeight() / 2.0;
    }

    private int getWidth() {
        return rightFrames[0].getWidth();
    }

    private int getHeight() {
        return rightFrames[0].getHeight();
    }
}
