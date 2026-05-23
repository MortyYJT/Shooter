package bulletbloom.core;

import bulletbloom.assets.AssetManager;
import bulletbloom.enemy.Enemy;
import bulletbloom.enemy.WaveController;
import bulletbloom.input.InputManager;
import bulletbloom.pickup.Coin;
import bulletbloom.player.Player;
import bulletbloom.save.SaveData;
import bulletbloom.save.SaveService;
import bulletbloom.shop.ShopController;
import bulletbloom.shop.ShopResult;
import bulletbloom.shop.ShopState;
import bulletbloom.shop.Wallet;
import bulletbloom.ui.HudRenderer;
import bulletbloom.ui.OverlayRenderer;
import bulletbloom.ui.ShopRenderer;
import bulletbloom.weapon.Weapon;
import bulletbloom.weapon.WeaponCatalog;
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
    private final Player player;
    private final List<Weapon> weapons;
    private final WaveController waveController;
    private final List<Enemy> enemies;
    private final List<Coin> coins;
    private final SaveService saveService;
    private final HudRenderer hudRenderer;
    private final OverlayRenderer overlayRenderer;
    private final ShopRenderer shopRenderer;
    private final ShopState shopState;
    private final Wallet wallet;
    private final ShopController shopController;
    private GameMode mode = GameMode.MENU;
    private int currentWeaponIndex;
    private String statusMessage = "";
    private int statusTimer;

    /**
     * Creates the initial game state.
     */
    public GameState() {
        this.screenBounds = new Rectangle(0, 0, GameConstants.SCREEN_WIDTH, GameConstants.SCREEN_HEIGHT);
        this.background = AssetManager.loadImage("/image/background/background_0.png");
        this.player = new Player(
                GameConstants.SCREEN_WIDTH / 2.0,
                GameConstants.SCREEN_HEIGHT / 2.0);
        this.weapons = WeaponCatalog.createDefaultWeapons();
        this.waveController = new WaveController();
        this.enemies = new ArrayList<>();
        this.coins = new ArrayList<>();
        this.saveService = new SaveService();
        this.hudRenderer = new HudRenderer();
        this.overlayRenderer = new OverlayRenderer();
        this.shopRenderer = new ShopRenderer();
        this.shopState = new ShopState();
        this.wallet = new Wallet();
        this.shopController = new ShopController(shopState, wallet, weapons);
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
        if (waveController.isInProgress()) {
            waveController.updateSpawning(enemies, screenBounds);
            for (Enemy enemy : enemies) {
                enemy.update(player, currentWeapon().getBullets());
            }
        } else if (input.consumeKeyPress(KeyEvent.VK_ENTER)) {
            waveController.startNextWave();
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
        hudRenderer.render(
                graphics,
                player,
                wallet.getBalance(),
                currentWeapon().getName(),
                waveController.getWave(),
                remainingEnemies(),
                coins.size(),
                statusMessage,
                statusTimer > 0);
        overlayRenderer.render(graphics, mode, waveController.isInProgress(), saveService.exists());
        if (mode == GameMode.SHOP) {
            shopRenderer.render(graphics, shopState, weapons, wallet.getBalance(), player);
        }
    }

    private void updateMode(InputManager input) {
        if (mode == GameMode.MENU && input.consumeKeyPress(KeyEvent.VK_ENTER)) {
            resetRun();
            mode = GameMode.PLAYING;
        } else if (mode == GameMode.MENU && input.consumeKeyPress(KeyEvent.VK_C) && saveService.exists()) {
            loadRun();
        } else if (mode == GameMode.PLAYING && input.consumeKeyPress(KeyEvent.VK_ESCAPE)) {
            mode = GameMode.PAUSED;
        } else if (mode == GameMode.PLAYING && !waveController.isInProgress() && input.consumeKeyPress(KeyEvent.VK_B)) {
            mode = GameMode.SHOP;
        } else if (mode == GameMode.PAUSED && input.consumeKeyPress(KeyEvent.VK_ESCAPE)) {
            mode = GameMode.PLAYING;
        } else if (mode == GameMode.PAUSED && input.consumeKeyPress(KeyEvent.VK_B)) {
            mode = GameMode.SHOP;
        } else if (mode == GameMode.PAUSED && input.consumeKeyPress(KeyEvent.VK_S)) {
            saveRun();
        } else if (mode == GameMode.SHOP) {
            updateShop(input);
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
        waveController.reset();
        currentWeaponIndex = 0;
        wallet.setBalance(0);
        shopState.reset();
        statusMessage = "";
        statusTimer = 0;
    }

    private void loadRun() {
        try {
            SaveData data = saveService.load();
            resetRun();
            waveController.restoreWave(data.wave());
            wallet.setBalance(data.money());
            player.setMaxHearts(data.maxHearts());
            player.setHearts(data.hearts());
            shopState.restoreUnlockedWeapons(data.unlockedWeapons());
            currentWeaponIndex = Math.max(0, Math.min(weapons.size() - 1, data.currentWeaponIndex()));
            if (!shopState.isWeaponUnlocked(currentWeaponIndex)) {
                currentWeaponIndex = 0;
            }
            mode = GameMode.PLAYING;
            setStatus("Loaded");
        } catch (IOException exception) {
            setStatus("Load failed");
        }
    }

    private void saveRun() {
        try {
            saveService.save(new SaveData(
                    waveController.getWave(),
                    wallet.getBalance(),
                    player.getHearts(),
                    player.getMaxHearts(),
                    currentWeaponIndex,
                    shopState.copyUnlockedWeapons()));
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
                coins.add(Coin.randomDrop(enemy.getCenterX(), enemy.getCenterY(), waveController.getWave()));
                iterator.remove();
            }
        }
    }

    private void updateCoins() {
        Iterator<Coin> iterator = coins.iterator();
        while (iterator.hasNext()) {
            Coin coin = iterator.next();
            wallet.add(coin.update(player));
            if (!coin.isActive()) {
                iterator.remove();
            }
        }
    }

    private void updateWeaponSelection(InputManager input) {
        if (input.consumeKeyPress(KeyEvent.VK_1) && shopState.isWeaponUnlocked(0)) {
            currentWeaponIndex = 0;
        } else if (input.consumeKeyPress(KeyEvent.VK_2) && weapons.size() > 1 && shopState.isWeaponUnlocked(1)) {
            currentWeaponIndex = 1;
        } else if (input.consumeKeyPress(KeyEvent.VK_3) && weapons.size() > 2 && shopState.isWeaponUnlocked(2)) {
            currentWeaponIndex = 2;
        } else if (input.consumeKeyPress(KeyEvent.VK_4) && weapons.size() > 3 && shopState.isWeaponUnlocked(3)) {
            currentWeaponIndex = 3;
        }
    }

    private void updateShop(InputManager input) {
        if (input.consumeKeyPress(KeyEvent.VK_ESCAPE) || input.consumeKeyPress(KeyEvent.VK_B)) {
            mode = GameMode.PAUSED;
            return;
        }
        if (input.consumeKeyPress(KeyEvent.VK_H)) {
            applyShopResult(shopController.buyHeal(player));
        }
        if (input.consumeKeyPress(KeyEvent.VK_M)) {
            applyShopResult(shopController.buyMaxHeart(player));
        }
        buyWeaponIfRequested(input, KeyEvent.VK_2, 1);
        buyWeaponIfRequested(input, KeyEvent.VK_3, 2);
        buyWeaponIfRequested(input, KeyEvent.VK_4, 3);
    }

    private void buyWeaponIfRequested(InputManager input, int keyCode, int weaponIndex) {
        if (!input.consumeKeyPress(keyCode)) {
            return;
        }
        applyShopResult(shopController.buyOrEquipWeapon(weaponIndex));
    }

    private void applyShopResult(ShopResult result) {
        if (result.hasSelectedWeapon()) {
            currentWeaponIndex = result.selectedWeaponIndex();
        }
        setStatus(result.message());
    }

    private Weapon currentWeapon() {
        return weapons.get(currentWeaponIndex);
    }

    private int remainingEnemies() {
        return waveController.remainingEnemies(enemies.size());
    }
}
