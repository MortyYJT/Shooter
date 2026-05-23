package bulletbloom.app;

import bulletbloom.core.GameConstants;
import javax.swing.Timer;

/**
 * Fixed-delay Swing game loop that drives update and repaint ticks.
 */
public final class GameLoop {
    private final Timer timer;

    /**
     * Creates a game loop.
     *
     * @param tick callback invoked once per loop frame
     */
    public GameLoop(Runnable tick) {
        int delayMillis = Math.max(1, 1000 / GameConstants.TARGET_FPS);
        this.timer = new Timer(delayMillis, event -> tick.run());
    }

    /**
     * Starts the game loop.
     */
    public void start() {
        timer.start();
    }

    /**
     * Stops the game loop.
     */
    public void stop() {
        timer.stop();
    }
}
