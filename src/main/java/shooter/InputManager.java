package shooter;

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
    private final Point mousePosition = new Point();
    private boolean leftMouseDown;
    private boolean leftMouseClicked;

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
     * Gets the last known mouse location inside the game panel.
     *
     * @return copy of the current mouse position
     */
    public Point getMousePosition() {
        return new Point(mousePosition);
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
        mousePosition.setLocation(event.getPoint());
    }

    @Override
    public void mouseReleased(MouseEvent event) {
        if (event.getButton() == MouseEvent.BUTTON1) {
            leftMouseDown = false;
        }
        mousePosition.setLocation(event.getPoint());
    }

    @Override
    public void mouseEntered(MouseEvent event) {
        mousePosition.setLocation(event.getPoint());
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
        mousePosition.setLocation(event.getPoint());
    }
}
