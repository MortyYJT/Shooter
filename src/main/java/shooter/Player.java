package shooter;

import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.event.KeyEvent;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;

/**
 * Player entity with movement and basic idle/walking animation.
 */
public final class Player {
    private static final int WALK_FRAME_COUNT = 5;
    private static final int IDLE_FRAME_COUNT = 3;
    private static final int WALK_FRAME_INTERVAL = 8;
    private static final int IDLE_FRAME_INTERVAL = 60;

    private final BufferedImage[] walkFrames = new BufferedImage[WALK_FRAME_COUNT];
    private final BufferedImage[] idleFrames = new BufferedImage[IDLE_FRAME_COUNT];
    private double x;
    private double y;
    private double speed;
    private Direction facing = Direction.LEFT;
    private int walkFrame;
    private int walkTimer;
    private int idleFrame;
    private int idleTimer;
    private boolean moving;

    /**
     * Creates a player at a world position.
     *
     * @param x initial x coordinate
     * @param y initial y coordinate
     */
    public Player(double x, double y) {
        this.x = x;
        this.y = y;
        this.speed = GameConstants.PLAYER_SPEED;
        loadAssets();
    }

    /**
     * Updates movement, facing direction, animation frame, and screen bounds.
     *
     * @param input current input state
     * @param bounds playable screen bounds
     */
    public void update(InputManager input, Rectangle bounds) {
        double dx = 0;
        double dy = 0;

        if (input.isKeyDown(KeyEvent.VK_A) || input.isKeyDown(KeyEvent.VK_LEFT)) {
            dx -= 1;
            facing = Direction.LEFT;
        }
        if (input.isKeyDown(KeyEvent.VK_D) || input.isKeyDown(KeyEvent.VK_RIGHT)) {
            dx += 1;
            facing = Direction.RIGHT;
        }
        if (input.isKeyDown(KeyEvent.VK_W) || input.isKeyDown(KeyEvent.VK_UP)) {
            dy -= 1;
        }
        if (input.isKeyDown(KeyEvent.VK_S) || input.isKeyDown(KeyEvent.VK_DOWN)) {
            dy += 1;
        }

        moving = dx != 0 || dy != 0;
        if (moving) {
            double length = Math.sqrt(dx * dx + dy * dy);
            x += dx / length * speed;
            y += dy / length * speed;
        }

        clampTo(bounds);
        updateAnimation(moving);
    }

    /**
     * Draws the player sprite at its current position.
     *
     * @param graphics active graphics context
     */
    public void draw(Graphics2D graphics) {
        BufferedImage frame = currentFrame();
        if (facing == Direction.RIGHT) {
            graphics.drawImage(frame, (int) Math.round(x), (int) Math.round(y), null);
            return;
        }

        AffineTransform original = graphics.getTransform();
        graphics.translate(x + GameConstants.PLAYER_WIDTH, y);
        graphics.scale(-1, 1);
        graphics.drawImage(frame, 0, 0, null);
        graphics.setTransform(original);
    }

    /**
     * Gets the x coordinate of the player's top-left corner.
     *
     * @return current x coordinate
     */
    public double getX() {
        return x;
    }

    /**
     * Gets the y coordinate of the player's top-left corner.
     *
     * @return current y coordinate
     */
    public double getY() {
        return y;
    }

    /**
     * Gets the x coordinate of the player's center point.
     *
     * @return current center x coordinate
     */
    public double getCenterX() {
        return x + GameConstants.PLAYER_WIDTH / 2.0;
    }

    /**
     * Gets the y coordinate of the player's center point.
     *
     * @return current center y coordinate
     */
    public double getCenterY() {
        return y + GameConstants.PLAYER_HEIGHT / 2.0;
    }

    private void loadAssets() {
        for (int index = 0; index < WALK_FRAME_COUNT; index++) {
            walkFrames[index] = AssetManager.loadImage(
                    "/image/player/player_walking/Lumine_walking_" + index + ".png");
        }
        for (int index = 0; index < IDLE_FRAME_COUNT; index++) {
            idleFrames[index] = AssetManager.loadImage(
                    "/image/player/player_breath/Lumine_breath_" + index + ".png");
        }
    }

    private void updateAnimation(boolean moving) {
        if (moving) {
            walkTimer++;
            if (walkTimer >= WALK_FRAME_INTERVAL) {
                walkTimer = 0;
                walkFrame = (walkFrame + 1) % WALK_FRAME_COUNT;
            }
            return;
        }

        idleTimer++;
        if (idleTimer >= IDLE_FRAME_INTERVAL) {
            idleTimer = 0;
            idleFrame = (idleFrame + 1) % IDLE_FRAME_COUNT;
        }
    }

    private BufferedImage currentFrame() {
        return moving ? walkFrames[walkFrame] : idleFrames[idleFrame];
    }

    private void clampTo(Rectangle bounds) {
        x = Math.max(bounds.getMinX(), Math.min(x, bounds.getMaxX() - GameConstants.PLAYER_WIDTH));
        y = Math.max(bounds.getMinY(), Math.min(y, bounds.getMaxY() - GameConstants.PLAYER_HEIGHT));
    }
}
