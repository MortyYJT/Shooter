package bulletbloom;

/**
 * Serializable snapshot of the Java game state.
 *
 * @param wave current wave number
 * @param money collected money
 * @param hearts current player hearts
 * @param maxHearts maximum player hearts
 * @param currentWeaponIndex selected weapon slot
 * @param unlockedWeapons unlocked weapon slots
 */
public record SaveData(int wave, int money, int hearts, int maxHearts, int currentWeaponIndex, boolean[] unlockedWeapons) {
}
