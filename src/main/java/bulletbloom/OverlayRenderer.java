package bulletbloom;

import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;

/**
 * Renders modal overlays such as menu, pause, game over, and wave clear.
 */
public final class OverlayRenderer {
    private static final Color SCRIM = new Color(0, 0, 0, 150);
    private static final Font TITLE_FONT = new Font(Font.SANS_SERIF, Font.BOLD, 42);
    private static final Font BODY_FONT = new Font(Font.SANS_SERIF, Font.BOLD, 22);

    /**
     * Creates an overlay renderer.
     */
    public OverlayRenderer() {
    }

    /**
     * Draws the current overlay when gameplay should be visually gated.
     *
     * @param graphics active graphics context
     * @param mode current game mode
     * @param waveInProgress whether the current wave is still active
     * @param hasSave whether a continue save is available
     */
    public void render(Graphics2D graphics, GameMode mode, boolean waveInProgress, boolean hasSave) {
        if (mode == GameMode.PLAYING && waveInProgress) {
            return;
        }

        graphics.setColor(SCRIM);
        graphics.fillRect(0, 0, GameConstants.SCREEN_WIDTH, GameConstants.SCREEN_HEIGHT);

        if (mode == GameMode.MENU) {
            drawCentered(graphics, "Bullet Bloom", -48, TITLE_FONT, Color.WHITE);
            drawCentered(graphics, "Press Enter", 8, BODY_FONT, Color.WHITE);
            if (hasSave) {
                drawCentered(graphics, "Press C to Continue", 40, BODY_FONT, Color.WHITE);
            }
        } else if (mode == GameMode.PAUSED) {
            drawCentered(graphics, "Paused", -24, TITLE_FONT, Color.WHITE);
            drawCentered(graphics, "Esc Resume    B Shop    S Save", 28, BODY_FONT, Color.WHITE);
        } else if (mode == GameMode.GAME_OVER) {
            drawCentered(graphics, "Game Over", -24, TITLE_FONT, Color.RED);
            drawCentered(graphics, "Press R to Restart", 28, BODY_FONT, Color.WHITE);
        } else if (mode == GameMode.PLAYING) {
            drawCentered(graphics, "Wave Clear", -24, TITLE_FONT, Color.WHITE);
            drawCentered(graphics, "Enter Next Wave    B Shop", 28, BODY_FONT, Color.WHITE);
        }
    }

    private void drawCentered(Graphics2D graphics, String text, int yOffset, Font font, Color color) {
        Font previousFont = graphics.getFont();
        graphics.setFont(font);
        graphics.setColor(color);
        FontMetrics metrics = graphics.getFontMetrics();
        int x = (GameConstants.SCREEN_WIDTH - metrics.stringWidth(text)) / 2;
        int y = GameConstants.SCREEN_HEIGHT / 2 + yOffset;
        graphics.drawString(text, x, y);
        graphics.setFont(previousFont);
    }
}
