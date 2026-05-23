package shooter;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.event.KeyEvent;
import java.awt.image.BufferedImage;
import java.io.IOException;
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
    private final List<Weapon> weapons;
    private final EnemySpawner enemySpawner;
    private final List<Enemy> enemies;
    private final List<Coin> coins;
    private final SaveService saveService;
    private GameMode mode = GameMode.MENU;
    private int currentWeaponIndex;
    private int money;
    private int wave = 1;
    private boolean waveInProgress = true;
    private String statusMessage = "";
    private int statusTimer;

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
        this.weapons = createWeapons();
        this.enemySpawner = new EnemySpawner();
        this.enemies = new ArrayList<>();
        this.coins = new ArrayList<>();
        this.saveService = new SaveService();
    }

    /**
     * Advances the game state by one frame.
     *
     * @param input current input state
     */
    public void update(InputManager input) {
        updateMode(input);
        if (mode != GameMode.PLAYING) {
            return;
        }

        updateWeaponSelection(input);
        player.update(input, screenBounds);
        currentWeapon().update(player, input, screenBounds);
        if (waveInProgress) {
            enemySpawner.update(enemies, screenBounds, wave);
            for (Enemy enemy : enemies) {
                enemy.update(player, currentWeapon().getBullets());
            }
            if (enemySpawner.isWaveComplete(wave, enemies)) {
                waveInProgress = false;
            }
        } else if (input.consumeKeyPress(KeyEvent.VK_ENTER)) {
            wave++;
            waveInProgress = true;
        }
        collectDefeatedEnemies();
        updateCoins();
        if (!player.isAlive()) {
            mode = GameMode.GAME_OVER;
        }
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
        currentWeapon().draw(graphics, player);
        for (Enemy enemy : enemies) {
            enemy.draw(graphics);
        }
        for (Coin coin : coins) {
            coin.draw(graphics);
        }
        drawHud(graphics);
        drawDebugText(graphics);
        drawOverlay(graphics);
    }

    private void updateMode(InputManager input) {
        if (mode == GameMode.MENU && input.consumeKeyPress(KeyEvent.VK_ENTER)) {
            resetRun();
            mode = GameMode.PLAYING;
        } else if (mode == GameMode.MENU && input.consumeKeyPress(KeyEvent.VK_C) && saveService.exists()) {
            loadRun();
        } else if (mode == GameMode.PLAYING && input.consumeKeyPress(KeyEvent.VK_ESCAPE)) {
            mode = GameMode.PAUSED;
        } else if (mode == GameMode.PAUSED && input.consumeKeyPress(KeyEvent.VK_ESCAPE)) {
            mode = GameMode.PLAYING;
        } else if (mode == GameMode.PAUSED && input.consumeKeyPress(KeyEvent.VK_S)) {
            saveRun();
        } else if (mode == GameMode.GAME_OVER && input.consumeKeyPress(KeyEvent.VK_R)) {
            resetRun();
            mode = GameMode.PLAYING;
        }
        if (statusTimer > 0) {
            statusTimer--;
        }
    }

    private void resetRun() {
        player.reset(GameConstants.SCREEN_WIDTH / 2.0, GameConstants.SCREEN_HEIGHT / 2.0);
        enemies.clear();
        coins.clear();
        enemySpawner.reset();
        currentWeaponIndex = 0;
        money = 0;
        wave = 1;
        waveInProgress = true;
        statusMessage = "";
        statusTimer = 0;
    }

    private void loadRun() {
        try {
            SaveData data = saveService.load();
            resetRun();
            wave = Math.max(1, data.wave());
            money = Math.max(0, data.money());
            player.setHearts(data.hearts());
            currentWeaponIndex = Math.max(0, Math.min(weapons.size() - 1, data.currentWeaponIndex()));
            mode = GameMode.PLAYING;
            setStatus("Loaded");
        } catch (IOException exception) {
            setStatus("Load failed");
        }
    }

    private void saveRun() {
        try {
            saveService.save(new SaveData(wave, money, player.getHearts(), currentWeaponIndex));
            setStatus("Saved");
        } catch (IOException exception) {
            setStatus("Save failed");
        }
    }

    private void setStatus(String message) {
        statusMessage = message;
        statusTimer = 180;
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
        graphics.drawString(currentWeapon().getName(), coinX + 160, y + 32);
        graphics.drawString("Wave " + wave, coinX + 260, y + 32);
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
        graphics.drawString(
                "Remaining: " + (enemySpawner.getRemainingToSpawn(wave) + enemies.size()),
                16,
                104);
        if (statusTimer > 0) {
            graphics.drawString(statusMessage, 16, 124);
        }
    }

    private void drawOverlay(Graphics2D graphics) {
        if (mode == GameMode.PLAYING && waveInProgress) {
            return;
        }

        graphics.setColor(new Color(0, 0, 0, 150));
        graphics.fillRect(0, 0, GameConstants.SCREEN_WIDTH, GameConstants.SCREEN_HEIGHT);
        graphics.setColor(Color.WHITE);
        if (mode == GameMode.MENU) {
            graphics.drawString("Bullet Bloom", GameConstants.SCREEN_WIDTH / 2 - 54, GameConstants.SCREEN_HEIGHT / 2 - 24);
            graphics.drawString("Press Enter", GameConstants.SCREEN_WIDTH / 2 - 42, GameConstants.SCREEN_HEIGHT / 2 + 8);
            if (saveService.exists()) {
                graphics.drawString("Press C", GameConstants.SCREEN_WIDTH / 2 - 28, GameConstants.SCREEN_HEIGHT / 2 + 40);
            }
        } else if (mode == GameMode.PAUSED) {
            graphics.drawString("Paused", GameConstants.SCREEN_WIDTH / 2 - 24, GameConstants.SCREEN_HEIGHT / 2 - 8);
            graphics.drawString("Press Esc", GameConstants.SCREEN_WIDTH / 2 - 34, GameConstants.SCREEN_HEIGHT / 2 + 24);
            graphics.drawString("Press S", GameConstants.SCREEN_WIDTH / 2 - 28, GameConstants.SCREEN_HEIGHT / 2 + 56);
        } else if (mode == GameMode.GAME_OVER) {
            graphics.setColor(Color.RED);
            graphics.drawString("Game Over", GameConstants.SCREEN_WIDTH / 2 - 38, GameConstants.SCREEN_HEIGHT / 2 - 8);
            graphics.setColor(Color.WHITE);
            graphics.drawString("Press R", GameConstants.SCREEN_WIDTH / 2 - 28, GameConstants.SCREEN_HEIGHT / 2 + 24);
        } else if (mode == GameMode.PLAYING) {
            graphics.drawString("Wave Clear", GameConstants.SCREEN_WIDTH / 2 - 38, GameConstants.SCREEN_HEIGHT / 2 - 8);
            graphics.drawString("Press Enter", GameConstants.SCREEN_WIDTH / 2 - 42, GameConstants.SCREEN_HEIGHT / 2 + 24);
        }
    }

    private void updateWeaponSelection(InputManager input) {
        if (input.consumeKeyPress(KeyEvent.VK_1)) {
            currentWeaponIndex = 0;
        } else if (input.consumeKeyPress(KeyEvent.VK_2) && weapons.size() > 1) {
            currentWeaponIndex = 1;
        } else if (input.consumeKeyPress(KeyEvent.VK_3) && weapons.size() > 2) {
            currentWeaponIndex = 2;
        } else if (input.consumeKeyPress(KeyEvent.VK_4) && weapons.size() > 3) {
            currentWeaponIndex = 3;
        }
    }

    private Weapon currentWeapon() {
        return weapons.get(currentWeaponIndex);
    }

    private List<Weapon> createWeapons() {
        List<Weapon> loadedWeapons = new ArrayList<>();
        loadedWeapons.add(new Pistol());
        loadedWeapons.add(new GunWeapon(
                "AK-47",
                "/image/weapon/weapon_AK-47.png",
                "/image/weapon/bullet_0.png",
                20,
                100,
                70,
                true));
        loadedWeapons.add(new GunWeapon(
                "Shotgun",
                "/image/weapon/Shotgun.png",
                "/image/weapon/Bullet_fire.png",
                56,
                50,
                24,
                false,
                -10,
                -5,
                -2,
                0,
                2,
                5,
                10));
        loadedWeapons.add(new GunWeapon(
                "AWP",
                "/image/weapon/AWP.png",
                "/image/weapon/Bullet_Yellow.png",
                120,
                120,
                300,
                false));
        return loadedWeapons;
    }
}
