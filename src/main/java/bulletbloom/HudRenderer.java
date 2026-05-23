package bulletbloom;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;

/**
 * Renders the in-game heads-up display.
 */
public final class HudRenderer {
    private final BufferedImage fullHeart;
    private final BufferedImage emptyHeart;
    private final BufferedImage coinIcon;

    /**
     * Loads HUD resources.
     */
    public HudRenderer() {
        this.fullHeart = AssetManager.loadImage("/image/ui/heart_full.png");
        this.emptyHeart = AssetManager.loadImage("/image/ui/heart_empty.png");
        this.coinIcon = AssetManager.loadImage("/image/ui/coin_4.png");
    }

    /**
     * Draws player health, money, weapon, wave, and status information.
     *
     * @param graphics active graphics context
     * @param player current player state
     * @param money collected money
     * @param weaponName selected weapon name
     * @param wave current wave number
     * @param remainingEnemies active plus unspawned enemies in the wave
     * @param activeCoins active coin count
     * @param statusMessage transient status text
     * @param showStatus whether the status text should be shown
     */
    public void render(
            Graphics2D graphics,
            Player player,
            int money,
            String weaponName,
            int wave,
            int remainingEnemies,
            int activeCoins,
            String statusMessage,
            boolean showStatus) {
        int x = 20;
        int y = 20;
        for (int index = 0; index < player.getMaxHearts(); index++) {
            BufferedImage image = index < player.getHearts() ? fullHeart : emptyHeart;
            graphics.drawImage(image, x + index * 48, y, null);
        }

        int coinX = x + player.getMaxHearts() * 48 + 40;
        graphics.drawImage(coinIcon, coinX, y, null);
        graphics.setColor(Color.BLACK);
        graphics.drawString("x " + money, coinX + 50, y + 32);
        graphics.drawString(weaponName, coinX + 160, y + 32);
        graphics.drawString("Wave " + wave, coinX + 260, y + 32);
        graphics.drawString("Remaining " + remainingEnemies, coinX + 350, y + 32);
        graphics.drawString("Coins " + activeCoins, coinX + 470, y + 32);

        if (showStatus) {
            graphics.drawString(statusMessage, 20, y + 72);
        }
    }
}
