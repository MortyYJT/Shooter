package bulletbloom;

import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.util.Random;

/**
 * Animated coin dropped by defeated enemies.
 */
public final class Coin {
    private static final BufferedImage[] FRAMES = loadFrames();
    private static final int FRAME_INTERVAL = 20;
    private static final double ATTRACT_SPEED = 10.0;
    private static final double COLLECT_DISTANCE = 75.0;
    private static final Random RANDOM = new Random();

    private double x;
    private double y;
    private final int value;
    private boolean active = true;
    private int frame;
    private int frameTimer;

    /**
     * Creates a coin at a world position.
     *
     * @param x initial x coordinate
     * @param y initial y coordinate
     * @param value money value awarded on collection
     */
    public Coin(double x, double y, int value) {
        this.x = x;
        this.y = y;
        this.value = value;
    }

    /**
     * Creates a randomly valued coin using the original 2-4 value range.
     *
     * @param x initial x coordinate
     * @param y initial y coordinate
     * @return new coin
     */
    public static Coin randomDrop(double x, double y) {
        return new Coin(x, y, 2 + RANDOM.nextInt(3));
    }

    /**
     * Updates animation and attraction toward the player.
     *
     * @param player player collecting the coin
     * @return collected value, or {@code 0} when not collected
     */
    public int update(Player player) {
        if (!active) {
            return 0;
        }

        updateAnimation();
        double dx = player.getCenterX() - x;
        double dy = player.getCenterY() - y;
        double distance = Math.sqrt(dx * dx + dy * dy);
        if (distance > 1) {
            x += dx / distance * ATTRACT_SPEED;
            y += dy / distance * ATTRACT_SPEED;
        }
        if (distance < COLLECT_DISTANCE) {
            active = false;
            return value;
        }
        return 0;
    }

    /**
     * Draws the active coin frame.
     *
     * @param graphics active graphics context
     */
    public void draw(Graphics2D graphics) {
        if (active) {
            graphics.drawImage(FRAMES[frame], (int) Math.round(x), (int) Math.round(y), null);
        }
    }

    /**
     * Checks whether the coin is still active.
     *
     * @return {@code true} until the coin is collected
     */
    public boolean isActive() {
        return active;
    }

    private void updateAnimation() {
        frameTimer++;
        if (frameTimer >= FRAME_INTERVAL) {
            frameTimer = 0;
            frame = (frame + 1) % FRAMES.length;
        }
    }

    private static BufferedImage[] loadFrames() {
        BufferedImage[] frames = new BufferedImage[10];
        for (int index = 0; index < frames.length; index++) {
            frames[index] = AssetManager.loadImage("/image/ui/coin" + (index + 1) + ".png");
        }
        return frames;
    }
}
