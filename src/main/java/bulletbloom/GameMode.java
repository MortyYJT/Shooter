package bulletbloom;

/**
 * High-level screen mode for the Java game flow.
 */
public enum GameMode {
    /** Title screen before gameplay starts. */
    MENU,

    /** Active gameplay update and render loop. */
    PLAYING,

    /** Frozen gameplay view with pause overlay. */
    PAUSED,

    /** Shop overlay between combat decisions. */
    SHOP,

    /** Player defeat screen. */
    GAME_OVER
}
