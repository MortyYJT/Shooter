package shooter;

import java.awt.Graphics2D;
import java.awt.image.BufferedImage;

/**
 * Minimal smoke test for the stage-one Java port.
 */
public final class GameSmokeTest {
    private GameSmokeTest() {
        throw new IllegalStateException("Test entry point");
    }

    /**
     * Verifies that core resources can load and the game state can render once.
     *
     * @param args command-line arguments, currently unused
     */
    public static void main(String[] args) {
        BufferedImage background = AssetManager.loadImage("/image/background/background_0.png");
        if (background.getWidth() <= 64 || background.getHeight() <= 64) {
            throw new IllegalStateException("Background asset did not load correctly");
        }

        GameState state = new GameState();
        BufferedImage frame = new BufferedImage(
                GameConstants.SCREEN_WIDTH,
                GameConstants.SCREEN_HEIGHT,
                BufferedImage.TYPE_INT_ARGB);
        Graphics2D graphics = frame.createGraphics();
        state.render(graphics);
        graphics.dispose();
    }
}
