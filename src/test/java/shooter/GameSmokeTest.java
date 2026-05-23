package shooter;

import java.awt.Canvas;
import java.awt.Graphics2D;
import java.awt.event.MouseEvent;
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
        InputManager input = new InputManager();
        Canvas source = new Canvas();
        input.mouseMoved(new MouseEvent(
                source,
                MouseEvent.MOUSE_MOVED,
                System.currentTimeMillis(),
                0,
                1000,
                600,
                0,
                false));
        input.mousePressed(new MouseEvent(
                source,
                MouseEvent.MOUSE_PRESSED,
                System.currentTimeMillis(),
                0,
                1000,
                600,
                1,
                false,
                MouseEvent.BUTTON1));
        state.update(input);
        for (int index = 0; index < 65; index++) {
            state.update(input);
        }

        BufferedImage frame = new BufferedImage(
                GameConstants.SCREEN_WIDTH,
                GameConstants.SCREEN_HEIGHT,
                BufferedImage.TYPE_INT_ARGB);
        Graphics2D graphics = frame.createGraphics();
        state.render(graphics);
        graphics.dispose();
    }
}
