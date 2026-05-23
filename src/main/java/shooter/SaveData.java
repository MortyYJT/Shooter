package shooter;

/**
 * Serializable snapshot of the Java game state.
 *
 * @param wave current wave number
 * @param money collected money
 * @param hearts current player hearts
 * @param currentWeaponIndex selected weapon slot
 */
public record SaveData(int wave, int money, int hearts, int currentWeaponIndex) {
}
