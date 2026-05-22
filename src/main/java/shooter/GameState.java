package shooter;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;

/**
 * Owns the active gameplay state for the first Java migration stage.
 */
public final class GameState {
    private final Rectangle screenBounds;
    private final BufferedImage background;
    private final Player player;
    private final Weapon weapon;

    /**
     * Creates the initial game state.
     */
    public GameState() {
        this.screenBounds = new Rectangle(0, 0, GameConstants.SCREEN_WIDTH, GameConstants.SCREEN_HEIGHT);
        this.background = AssetManager.loadImage("/image/background/background_0.png");
        this.player = new Player(
                GameConstants.SCREEN_WIDTH / 2.0,
                GameConstants.SCREEN_HEIGHT / 2.0);
        this.weapon = new Pistol();
    }

    /**
     * Advances the game state by one frame.
     *
     * @param input current input state
     */
    public void update(InputManager input) {
        player.update(input, screenBounds);
        weapon.update(player, input, screenBounds);
    }

    /**
     * Draws the current frame.
     *
     * @param graphics active graphics context
     */
    public void render(Graphics2D graphics) {
        graphics.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
        AssetManager.drawScaled(
                graphics,
                background,
                0,
                0,
                GameConstants.SCREEN_WIDTH,
                GameConstants.SCREEN_HEIGHT);
        player.draw(graphics);
        weapon.draw(graphics, player);
        drawDebugText(graphics);
    }

    private void drawDebugText(Graphics2D graphics) {
        graphics.setColor(new Color(255, 255, 255, 210));
        graphics.drawString("Stage 1 Java port | Move: WASD / Arrow keys", 16, 24);
        graphics.drawString(
                "Player: " + Math.round(player.getX()) + ", " + Math.round(player.getY()),
                16,
                44);
    }
}
