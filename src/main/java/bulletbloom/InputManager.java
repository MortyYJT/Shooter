package bulletbloom;

import java.awt.Point;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.util.HashSet;
import java.util.Set;

/**
 * Tracks keyboard and mouse state for the current game window.
 */
public final class InputManager implements KeyListener, MouseListener, MouseMotionListener {
    private final Set<Integer> pressedKeys = new HashSet<>();
    private final Set<Integer> typedKeys = new HashSet<>();
    private final Point mousePosition = new Point();
    private int viewportX;
    private int viewportY;
    private double viewportScale = 1.0;
    private boolean leftMouseDown;
    private boolean leftMouseClicked;

    /**
     * Creates an empty input state tracker.
     */
    public InputManager() {
    }

    /**
     * Checks whether a key is currently held down.
     *
     * @param keyCode {@link KeyEvent} virtual key code
     * @return {@code true} when the key is pressed
     */
    public boolean isKeyDown(int keyCode) {
        return pressedKeys.contains(keyCode);
    }

    /**
     * Consumes a key press that occurred since the previous update.
     *
     * @param keyCode {@link KeyEvent} virtual key code
     * @return {@code true} once for each fresh key press
     */
    public boolean consumeKeyPress(int keyCode) {
        return typedKeys.remove(keyCode);
    }

    /**
     * Gets the last known mouse location inside the game panel.
     *
     * @return copy of the current mouse position
     */
    public Point getMousePosition() {
        return new Point(mousePosition);
    }

    /**
     * Updates the viewport transform used to map panel mouse coordinates into
     * the game's logical coordinate system.
     *
     * @param viewportX x offset of the rendered game viewport
     * @param viewportY y offset of the rendered game viewport
     * @param viewportScale scale applied to the logical game canvas
     */
    public void updateViewport(int viewportX, int viewportY, double viewportScale) {
        this.viewportX = viewportX;
        this.viewportY = viewportY;
        this.viewportScale = Math.max(0.0001, viewportScale);
    }

    /**
     * Checks whether the left mouse button is currently held down.
     *
     * @return {@code true} while the left mouse button is pressed
     */
    public boolean isLeftMouseDown() {
        return leftMouseDown;
    }

    /**
     * Consumes a left-click event captured since the previous update.
     *
     * @return {@code true} once for each left mouse press
     */
    public boolean consumeLeftMouseClick() {
        boolean clicked = leftMouseClicked;
        leftMouseClicked = false;
        return clicked;
    }

    @Override
    public void keyTyped(KeyEvent event) {
        // The game works from pressed/released state instead of typed characters.
    }

    @Override
    public void keyPressed(KeyEvent event) {
        if (!pressedKeys.contains(event.getKeyCode())) {
            typedKeys.add(event.getKeyCode());
        }
        pressedKeys.add(event.getKeyCode());
    }

    @Override
    public void keyReleased(KeyEvent event) {
        pressedKeys.remove(event.getKeyCode());
    }

    @Override
    public void mouseClicked(MouseEvent event) {
        // Press/release events provide more reliable frame-level state.
    }

    @Override
    public void mousePressed(MouseEvent event) {
        if (event.getButton() == MouseEvent.BUTTON1) {
            leftMouseDown = true;
            leftMouseClicked = true;
        }
        updateMousePosition(event);
    }

    @Override
    public void mouseReleased(MouseEvent event) {
        if (event.getButton() == MouseEvent.BUTTON1) {
            leftMouseDown = false;
        }
        updateMousePosition(event);
    }

    @Override
    public void mouseEntered(MouseEvent event) {
        updateMousePosition(event);
    }

    @Override
    public void mouseExited(MouseEvent event) {
        leftMouseDown = false;
    }

    @Override
    public void mouseDragged(MouseEvent event) {
        mouseMoved(event);
    }

    @Override
    public void mouseMoved(MouseEvent event) {
        updateMousePosition(event);
    }

    private void updateMousePosition(MouseEvent event) {
        int logicalX = (int) Math.round((event.getX() - viewportX) / viewportScale);
        int logicalY = (int) Math.round((event.getY() - viewportY) / viewportScale);
        logicalX = Math.max(0, Math.min(GameConstants.SCREEN_WIDTH, logicalX));
        logicalY = Math.max(0, Math.min(GameConstants.SCREEN_HEIGHT, logicalY));
        mousePosition.setLocation(logicalX, logicalY);
    }
}
