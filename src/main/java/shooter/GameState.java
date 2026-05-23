package shooter;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * Owns the active gameplay state for the first Java migration stage.
 */
public final class GameState {
    private final Rectangle screenBounds;
    private final BufferedImage background;
    private final BufferedImage fullHeart;
    private final BufferedImage emptyHeart;
    private final BufferedImage coinIcon;
    private final Player player;
    private final Weapon weapon;
    private final EnemySpawner enemySpawner;
    private final List<Enemy> enemies;
    private final List<Coin> coins;
    private int money;

    /**
     * Creates the initial game state.
     */
    public GameState() {
        this.screenBounds = new Rectangle(0, 0, GameConstants.SCREEN_WIDTH, GameConstants.SCREEN_HEIGHT);
        this.background = AssetManager.loadImage("/image/background/background_0.png");
        this.fullHeart = AssetManager.loadImage("/image/ui/heart_full.png");
        this.emptyHeart = AssetManager.loadImage("/image/ui/heart_empty.png");
        this.coinIcon = AssetManager.loadImage("/image/ui/coin_4.png");
        this.player = new Player(
                GameConstants.SCREEN_WIDTH / 2.0,
                GameConstants.SCREEN_HEIGHT / 2.0);
        this.weapon = new Pistol();
        this.enemySpawner = new EnemySpawner();
        this.enemies = new ArrayList<>();
        this.coins = new ArrayList<>();
    }

    /**
     * Advances the game state by one frame.
     *
     * @param input current input state
     */
    public void update(InputManager input) {
        player.update(input, screenBounds);
        weapon.update(player, input, screenBounds);
        enemySpawner.update(enemies, screenBounds);
        for (Enemy enemy : enemies) {
            enemy.update(player, weapon.getBullets());
        }
        collectDefeatedEnemies();
        updateCoins();
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
        for (Enemy enemy : enemies) {
            enemy.draw(graphics);
        }
        for (Coin coin : coins) {
            coin.draw(graphics);
        }
        drawHud(graphics);
        drawDebugText(graphics);
    }

    private void collectDefeatedEnemies() {
        Iterator<Enemy> iterator = enemies.iterator();
        while (iterator.hasNext()) {
            Enemy enemy = iterator.next();
            if (!enemy.isAlive()) {
                coins.add(Coin.randomDrop(enemy.getCenterX(), enemy.getCenterY()));
                iterator.remove();
            }
        }
    }

    private void updateCoins() {
        Iterator<Coin> iterator = coins.iterator();
        while (iterator.hasNext()) {
            Coin coin = iterator.next();
            money += coin.update(player);
            if (!coin.isActive()) {
                iterator.remove();
            }
        }
    }

    private void drawHud(Graphics2D graphics) {
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
    }

    private void drawDebugText(Graphics2D graphics) {
        graphics.setColor(new Color(255, 255, 255, 210));
        graphics.drawString("Bullet Bloom | Move: WASD / Arrow keys | Fire: Left click", 16, 24);
        graphics.drawString(
                "Player: " + Math.round(player.getX()) + ", " + Math.round(player.getY()),
                16,
                44);
        graphics.drawString(
                "HP: " + player.getHearts() + "/" + player.getMaxHearts() + " | Enemies: " + enemies.size(),
                16,
                64);
        graphics.drawString("Money: " + money + " | Coins: " + coins.size(), 16, 84);
    }
}
