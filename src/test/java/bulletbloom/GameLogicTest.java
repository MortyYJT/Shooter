package bulletbloom;

import java.awt.Canvas;
import java.awt.Rectangle;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.nio.file.Files;
import java.nio.file.Path;

/**
 * Lightweight deterministic tests for non-rendering game logic.
 */
public final class GameLogicTest {
    private GameLogicTest() {
        throw new IllegalStateException("Test entry point");
    }

    /**
     * Runs logic checks without requiring a game window.
     *
     * @param args command-line arguments, currently unused
     * @throws Exception if a test setup step fails
     */
    public static void main(String[] args) throws Exception {
        testGeometrySegmentIntersection();
        testInputConsumption();
        testSaveRoundTrip();
    }

    private static void testGeometrySegmentIntersection() {
        Rectangle rectangle = new Rectangle(10, 10, 20, 20);
        assertTrue(
                Geometry.segmentIntersectsRectangle(0, 20, 40, 20, rectangle),
                "segment crossing rectangle should hit");
        assertFalse(
                Geometry.segmentIntersectsRectangle(0, 0, 5, 5, rectangle),
                "segment outside rectangle should miss");
    }

    private static void testInputConsumption() {
        InputManager input = new InputManager();
        Canvas source = new Canvas();
        input.keyPressed(new KeyEvent(
                source,
                KeyEvent.KEY_PRESSED,
                System.currentTimeMillis(),
                0,
                KeyEvent.VK_1,
                KeyEvent.CHAR_UNDEFINED));
        assertTrue(input.consumeKeyPress(KeyEvent.VK_1), "fresh key press should be consumed");
        assertFalse(input.consumeKeyPress(KeyEvent.VK_1), "key press should only be consumed once");

        input.mousePressed(new MouseEvent(
                source,
                MouseEvent.MOUSE_PRESSED,
                System.currentTimeMillis(),
                0,
                10,
                20,
                1,
                false,
                MouseEvent.BUTTON1));
        assertTrue(input.isLeftMouseDown(), "left mouse button should be down after press");
        assertTrue(input.consumeLeftMouseClick(), "left mouse click should be consumed");
        assertFalse(input.consumeLeftMouseClick(), "left mouse click should only be consumed once");
    }

    private static void testSaveRoundTrip() throws Exception {
        Path tempDirectory = Files.createTempDirectory("bullet-bloom-save-test");
        Path savePath = tempDirectory.resolve("save.properties");
        SaveService saveService = new SaveService(savePath);
        SaveData saved = new SaveData(4, 320, 5, 2);
        saveService.save(saved);
        SaveData loaded = saveService.load();
        assertEquals(saved.wave(), loaded.wave(), "wave should round-trip");
        assertEquals(saved.money(), loaded.money(), "money should round-trip");
        assertEquals(saved.hearts(), loaded.hearts(), "hearts should round-trip");
        assertEquals(saved.currentWeaponIndex(), loaded.currentWeaponIndex(), "weapon slot should round-trip");
        Files.deleteIfExists(savePath);
        Files.deleteIfExists(tempDirectory);
    }

    private static void assertTrue(boolean value, String message) {
        if (!value) {
            throw new IllegalStateException(message);
        }
    }

    private static void assertFalse(boolean value, String message) {
        assertTrue(!value, message);
    }

    private static void assertEquals(int expected, int actual, String message) {
        if (expected != actual) {
            throw new IllegalStateException(message + ": expected " + expected + " but got " + actual);
        }
    }
}
