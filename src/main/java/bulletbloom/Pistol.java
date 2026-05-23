package bulletbloom;

import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * Pistol weapon migrated from the original DEagle implementation.
 */
public final class Pistol implements Weapon {
    private static final int FIRE_INTERVAL = 20;
    private static final int FIRE_FRAME_DURATION = 6;
    private static final int MUZZLE_FLASH_DURATION = 6;
    private static final double BULLET_SPEED = 50.0;
    private static final int BULLET_DAMAGE = 100;

    private final BufferedImage idleImage;
    private final BufferedImage firingImage;
    private final BufferedImage bulletImage;
    private final BufferedImage muzzleFlashImage;
    private final List<Bullet> bullets = new ArrayList<>();
    private int fireCooldown;
    private int fireFrameTimer;
    private int muzzleTimer;
    private double muzzleX;
    private double muzzleY;
    private double aimX = GameConstants.SCREEN_WIDTH / 2.0;
    private double aimY = GameConstants.SCREEN_HEIGHT / 2.0;

    /**
     * Loads pistol resources.
     */
    public Pistol() {
        this.idleImage = AssetManager.loadImage("/image/weapon/DEagle_0.png");
        this.firingImage = AssetManager.loadImage("/image/weapon/DEagle_1.png");
        this.bulletImage = AssetManager.loadImage("/image/weapon/bullet_0.png");
        this.muzzleFlashImage = AssetManager.loadImage("/image/weapon/muzzle_flash.png");
    }

    /**
     * Updates pistol cooldowns, handles one left-click shot, and advances active bullets.
     *
     * @param player player using the pistol
     * @param input current input state
     * @param bounds active play area
     */
    @Override
    public void update(Player player, InputManager input, Rectangle bounds) {
        if (fireCooldown > 0) {
            fireCooldown--;
        }
        if (fireFrameTimer > 0) {
            fireFrameTimer--;
        }
        if (muzzleTimer > 0) {
            muzzleTimer--;
        }

        Point mouse = input.getMousePosition();
        aimX = mouse.x;
        aimY = mouse.y;
        double angle = Math.atan2(aimY - player.getCenterY(), aimX - player.getCenterX());
        if (input.consumeLeftMouseClick() && fireCooldown == 0) {
            fire(angle, player);
        }

        Iterator<Bullet> iterator = bullets.iterator();
        while (iterator.hasNext()) {
            Bullet bullet = iterator.next();
            bullet.update(bounds);
            if (!bullet.isActive()) {
                iterator.remove();
            }
        }
    }

    /**
     * Draws the pistol, muzzle flash, and active bullets.
     *
     * @param graphics active graphics context
     * @param player player using the pistol
     */
    @Override
    public void draw(Graphics2D graphics, Player player) {
        double weaponX = player.getX() + 15.0;
        double weaponY = player.getY() + 25.0;
        double angle = Math.atan2(aimY - weaponY, aimX - weaponX);

        drawWeapon(graphics, currentImage(), weaponX, weaponY, angle, aimX < player.getCenterX());
        if (muzzleTimer > 0) {
            drawRotated(graphics, muzzleFlashImage, muzzleX, muzzleY, angle, false);
        }
        for (Bullet bullet : bullets) {
            bullet.draw(graphics);
        }
    }

    /**
     * Gets active bullets fired by the pistol.
     *
     * @return mutable bullet list owned by this pistol
     */
    @Override
    public List<Bullet> getBullets() {
        return bullets;
    }

    /**
     * Gets the pistol display name.
     *
     * @return weapon name
     */
    @Override
    public String getName() {
        return "Pistol";
    }

    private void fire(double angle, Player player) {
        fireCooldown = FIRE_INTERVAL;
        fireFrameTimer = FIRE_FRAME_DURATION;
        muzzleTimer = MUZZLE_FLASH_DURATION;
        muzzleX = player.getCenterX() + Math.cos(angle) * 45.0;
        muzzleY = player.getCenterY() + Math.sin(angle) * 45.0;
        bullets.add(new Bullet(player.getCenterX(), player.getCenterY(), angle, BULLET_SPEED, BULLET_DAMAGE, bulletImage));
    }

    private BufferedImage currentImage() {
        return fireFrameTimer > 0 ? firingImage : idleImage;
    }

    private void drawWeapon(
            Graphics2D graphics,
            BufferedImage image,
            double x,
            double y,
            double angle,
            boolean flipped) {
        drawRotated(graphics, image, x, y, angle, flipped);
    }

    private void drawRotated(
            Graphics2D graphics,
            BufferedImage image,
            double x,
            double y,
            double angle,
            boolean flipped) {
        AffineTransform original = graphics.getTransform();
        graphics.translate(x, y);
        graphics.rotate(angle, image.getWidth() / 2.0, image.getHeight() / 2.0);
        if (flipped) {
            graphics.translate(image.getWidth(), 0);
            graphics.scale(-1, 1);
        }
        graphics.drawImage(image, 0, 0, null);
        graphics.setTransform(original);
    }
}
