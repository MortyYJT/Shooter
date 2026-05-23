package bulletbloom.app;

import bulletbloom.core.GameConstants;
import bulletbloom.core.GameState;
import bulletbloom.input.InputManager;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import javax.swing.JPanel;

/**
 * Swing drawing surface for the Java port.
 */
public final class GamePanel extends JPanel {
    private static final long serialVersionUID = 1L;

    /** Input tracker attached to the panel. */
    private final InputManager input;

    /** Mutable game state updated by the loop. */
    private final GameState gameState;

    /** Swing timer loop that advances gameplay. */
    private final GameLoop gameLoop;

    /** Fixed-size logical render target that is scaled to the panel. */
    private final BufferedImage frameBuffer;

    /**
     * Creates and wires the playable game panel.
     */
    public GamePanel() {
        this.input = new InputManager();
        this.gameState = new GameState();
        this.gameLoop = new GameLoop(this::tick);
        this.frameBuffer = new BufferedImage(
                GameConstants.SCREEN_WIDTH,
                GameConstants.SCREEN_HEIGHT,
                BufferedImage.TYPE_INT_ARGB);

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
        Graphics2D bufferGraphics = frameBuffer.createGraphics();
        try {
            gameState.render(bufferGraphics);
        } finally {
            bufferGraphics.dispose();
        }

        Viewport viewport = calculateViewport();
        input.updateViewport(viewport.x(), viewport.y(), viewport.scale());
        Graphics2D graphics2D = (Graphics2D) graphics;
        graphics2D.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
        graphics2D.drawImage(frameBuffer, viewport.x(), viewport.y(), viewport.width(), viewport.height(), null);
    }

    private void tick() {
        gameState.update(input);
        repaint();
    }

    private Viewport calculateViewport() {
        double scaleX = getWidth() / (double) GameConstants.SCREEN_WIDTH;
        double scaleY = getHeight() / (double) GameConstants.SCREEN_HEIGHT;
        double scale = Math.max(0.0001, Math.min(scaleX, scaleY));
        int width = Math.max(1, (int) Math.round(GameConstants.SCREEN_WIDTH * scale));
        int height = Math.max(1, (int) Math.round(GameConstants.SCREEN_HEIGHT * scale));
        int x = (getWidth() - width) / 2;
        int y = (getHeight() - height) / 2;
        return new Viewport(x, y, width, height, scale);
    }

    private record Viewport(int x, int y, int width, int height, double scale) {
    }
}
