package bulletbloom.ui;

import bulletbloom.core.GameConstants;
import bulletbloom.player.Player;
import bulletbloom.shop.ShopState;
import bulletbloom.weapon.Weapon;
import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.util.List;

/**
 * Renders shop purchase options.
 */
public final class ShopRenderer {
    private static final Color SCRIM = new Color(0, 0, 0, 170);
    private static final Font TITLE_FONT = new Font(Font.SANS_SERIF, Font.BOLD, 36);
    private static final Font BODY_FONT = new Font(Font.SANS_SERIF, Font.BOLD, 20);

    /**
     * Creates a shop renderer.
     */
    public ShopRenderer() {
    }

    /**
     * Draws the shop overlay.
     *
     * @param graphics active graphics context
     * @param shopState current shop state
     * @param weapons weapon slots
     * @param money available money
     * @param player current player state
     */
    public void render(Graphics2D graphics, ShopState shopState, List<Weapon> weapons, int money, Player player) {
        graphics.setColor(SCRIM);
        graphics.fillRect(0, 0, GameConstants.SCREEN_WIDTH, GameConstants.SCREEN_HEIGHT);

        drawCentered(graphics, "Shop", -170, TITLE_FONT, Color.WHITE);
        drawCentered(graphics, "Money: " + money, -130, BODY_FONT, Color.WHITE);
        drawCentered(graphics, "H: Heal  " + ShopState.HEAL_COST + "    M: Max HP  " + ShopState.MAX_HEART_COST,
                -80,
                BODY_FONT,
                Color.WHITE);
        drawCentered(graphics, "HP: " + player.getHearts() + "/" + player.getMaxHearts(), -50, BODY_FONT, Color.WHITE);
        drawCentered(graphics, "Press a weapon number to buy or equip", -18, BODY_FONT, Color.WHITE);

        int y = GameConstants.SCREEN_HEIGHT / 2 + 24;
        for (int index = 1; index < weapons.size(); index++) {
            String state = shopState.isWeaponUnlocked(index) ? "Unlocked" : shopState.getWeaponCost(index) + " coins";
            drawCentered(
                    graphics,
                    index + ": " + weapons.get(index).getName() + " - " + state,
                    y - GameConstants.SCREEN_HEIGHT / 2,
                    BODY_FONT,
                    Color.WHITE);
            y += 32;
        }
        drawCentered(graphics, "B or Esc: Close", 170, BODY_FONT, Color.WHITE);
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
