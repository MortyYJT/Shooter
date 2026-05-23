package bulletbloom.weapon;

import bulletbloom.assets.AssetManager;
import bulletbloom.core.GameConstants;
import bulletbloom.input.InputManager;
import bulletbloom.player.Player;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * Configurable firearm used for migrated non-pistol weapons.
 */
public final class GunWeapon implements Weapon {
    private final String name;
    private final BufferedImage weaponImage;
    private final BufferedImage bulletImage;
    private final BufferedImage muzzleFlashImage;
    private final List<Bullet> bullets = new ArrayList<>();
    private final int fireInterval;
    private final double bulletSpeed;
    private final int bulletDamage;
    private final boolean automatic;
    private final double[] spreadDegrees;
    private int fireCooldown;
    private int muzzleTimer;
    private double aimX = GameConstants.SCREEN_WIDTH / 2.0;
    private double aimY = GameConstants.SCREEN_HEIGHT / 2.0;
    private double muzzleX;
    private double muzzleY;

    /**
     * Creates a configured firearm.
     *
     * @param name display name
     * @param weaponPath classpath path for weapon sprite
     * @param bulletPath classpath path for bullet sprite
     * @param fireInterval cooldown in update ticks
     * @param bulletSpeed bullet speed per tick
     * @param bulletDamage bullet damage
     * @param automatic whether holding left mouse fires repeatedly
     * @param spreadDegrees pellet spread offsets in degrees
     */
    public GunWeapon(
            String name,
            String weaponPath,
            String bulletPath,
            int fireInterval,
            double bulletSpeed,
            int bulletDamage,
            boolean automatic,
            double... spreadDegrees) {
        this.name = name;
        this.weaponImage = AssetManager.loadImage(weaponPath);
        this.bulletImage = AssetManager.loadImage(bulletPath);
        this.muzzleFlashImage = AssetManager.loadImage("/image/weapon/muzzle_flash.png");
        this.fireInterval = fireInterval;
        this.bulletSpeed = bulletSpeed;
        this.bulletDamage = bulletDamage;
        this.automatic = automatic;
        this.spreadDegrees = spreadDegrees.length == 0 ? new double[] {0} : spreadDegrees;
    }

    @Override
    public void update(Player player, InputManager input, Rectangle bounds) {
        if (fireCooldown > 0) {
            fireCooldown--;
        }
        if (muzzleTimer > 0) {
            muzzleTimer--;
        }

        Point mouse = input.getMousePosition();
        aimX = mouse.x;
        aimY = mouse.y;
        boolean firing = automatic ? input.isLeftMouseDown() : input.consumeLeftMouseClick();
        if (firing && fireCooldown == 0) {
            fire(player);
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

    @Override
    public void draw(Graphics2D graphics, Player player) {
        double weaponX = player.getX();
        double weaponY = player.getY() + 32.0;
        double angle = Math.atan2(aimY - weaponY, aimX - weaponX);
        drawRotated(graphics, weaponImage, weaponX, weaponY, angle, aimX < player.getCenterX());

        if (muzzleTimer > 0) {
            drawRotated(graphics, muzzleFlashImage, muzzleX, muzzleY, angle, false);
        }
        for (Bullet bullet : bullets) {
            bullet.draw(graphics);
        }
    }

    @Override
    public List<Bullet> getBullets() {
        return bullets;
    }

    @Override
    public String getName() {
        return name;
    }

    private void fire(Player player) {
        double angle = Math.atan2(aimY - player.getCenterY(), aimX - player.getCenterX());
        fireCooldown = fireInterval;
        muzzleTimer = 4;
        muzzleX = player.getCenterX() + Math.cos(angle) * 45.0;
        muzzleY = player.getCenterY() + Math.sin(angle) * 45.0;
        for (double spread : spreadDegrees) {
            double pelletAngle = angle + Math.toRadians(spread);
            bullets.add(new Bullet(
                    player.getCenterX(),
                    player.getCenterY(),
                    pelletAngle,
                    bulletSpeed,
                    bulletDamage,
                    bulletImage));
        }
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
