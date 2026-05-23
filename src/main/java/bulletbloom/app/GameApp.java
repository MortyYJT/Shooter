package bulletbloom.app;

import java.awt.EventQueue;
import javax.swing.JFrame;
import javax.swing.WindowConstants;

/**
 * Application entry point for the Java rewrite of the shooter game.
 */
public final class GameApp {
    private GameApp() {
        throw new IllegalStateException("Application entry point");
    }

    /**
     * Starts the desktop game window.
     *
     * @param args command-line arguments, currently unused
     */
    public static void main(String[] args) {
        EventQueue.invokeLater(GameApp::createWindow);
    }

    private static void createWindow() {
        JFrame frame = new JFrame("Bullet Bloom");
        GamePanel panel = new GamePanel();
        frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        frame.setResizable(true);
        frame.setContentPane(panel);
        frame.pack();
        frame.setLocationRelativeTo(null);
        frame.setVisible(true);
        panel.start();
    }
}
