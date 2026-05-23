package bulletbloom;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import javax.swing.JPanel;

/**
 * Swing drawing surface for the Java port.
 */
public final class GamePanel extends JPanel {
    private final InputManager input;
    private final GameState gameState;
    private final GameLoop gameLoop;

    /**
     * Creates and wires the playable game panel.
     */
    public GamePanel() {
        this.input = new InputManager();
        this.gameState = new GameState();
        this.gameLoop = new GameLoop(this::tick);

        setPreferredSize(new Dimension(GameConstants.SCREEN_WIDTH, GameConstants.SCREEN_HEIGHT));
        setFocusable(true);
        addKeyListener(input);
        addMouseListener(input);
        addMouseMotionListener(input);
    }

    /**
     * Starts the panel's game loop.
     */
    public void start() {
        requestFocusInWindow();
        gameLoop.start();
    }

    @Override
    protected void paintComponent(Graphics graphics) {
        super.paintComponent(graphics);
        gameState.render((Graphics2D) graphics);
    }

    private void tick() {
        gameState.update(input);
        repaint();
    }
}
