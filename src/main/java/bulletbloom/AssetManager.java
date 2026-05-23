package bulletbloom;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.InputStream;
import javax.imageio.ImageIO;

/**
 * Loads image assets from {@code src/main/resources} through the classpath.
 */
public final class AssetManager {
    private AssetManager() {
        throw new IllegalStateException("Utility class");
    }

    /**
     * Loads an image from the classpath.
     *
     * @param path classpath-relative asset path, for example
     *             {@code /image/background/background_0.png}
     * @return decoded image, or a visible placeholder when the asset is missing
     */
    public static BufferedImage loadImage(String path) {
        try (InputStream input = AssetManager.class.getResourceAsStream(path)) {
            if (input == null) {
                return missingImage(path);
            }
            return ImageIO.read(input);
        } catch (IOException exception) {
            return missingImage(path);
        }
    }

    /**
     * Draws an image scaled to fill the target rectangle.
     *
     * @param graphics active graphics context
     * @param image image to draw
     * @param x target x coordinate
     * @param y target y coordinate
     * @param width target width
     * @param height target height
     */
    public static void drawScaled(Graphics2D graphics, Image image, int x, int y, int width, int height) {
        graphics.drawImage(image, x, y, width, height, null);
    }

    private static BufferedImage missingImage(String label) {
        BufferedImage image = new BufferedImage(64, 64, BufferedImage.TYPE_INT_ARGB);
        Graphics2D graphics = image.createGraphics();
        graphics.setColor(new Color(180, 40, 80));
        graphics.fillRect(0, 0, 64, 64);
        graphics.setColor(Color.WHITE);
        graphics.drawLine(0, 0, 63, 63);
        graphics.drawLine(63, 0, 0, 63);
        graphics.drawString("missing", 6, 30);
        graphics.dispose();
        System.err.println("Missing asset: " + label);
        return image;
    }
}
