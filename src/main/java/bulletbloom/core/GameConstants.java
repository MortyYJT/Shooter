package bulletbloom.core;

/**
 * Shared constants for the Java port of the shooter game.
 */
public final class GameConstants {
    /** Width of the original SplashKit window. */
    public static final int SCREEN_WIDTH = 1600;

    /** Height of the original SplashKit window. */
    public static final int SCREEN_HEIGHT = 1200;

    /** Target frames per second for update and render ticks. */
    public static final int TARGET_FPS = 120;

    /** Default player width used by the C++ version. */
    public static final int PLAYER_WIDTH = 48;

    /** Default player height used by the C++ version. */
    public static final int PLAYER_HEIGHT = 48;

    /** Baseline movement speed from the original player setup. */
    public static final double PLAYER_SPEED = 2.0;

    private GameConstants() {
        throw new IllegalStateException("Utility class");
    }
}
