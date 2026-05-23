package bulletbloom.shop;

/**
 * Result returned by a shop action.
 *
 * @param selectedWeaponIndex optional selected weapon index, or {@code -1}
 * @param message user-facing status message
 */
public record ShopResult(int selectedWeaponIndex, String message) {
    /**
     * Creates a message-only result.
     *
     * @param message status message
     * @return result without weapon selection
     */
    public static ShopResult message(String message) {
        return new ShopResult(-1, message);
    }

    /**
     * Creates a weapon equipment result.
     *
     * @param weaponIndex selected weapon slot
     * @param message status message
     * @return result with weapon selection
     */
    public static ShopResult equipped(int weaponIndex, String message) {
        return new ShopResult(weaponIndex, message);
    }

    /**
     * Checks whether this result selected a weapon.
     *
     * @return {@code true} when a weapon slot should be equipped
     */
    public boolean hasSelectedWeapon() {
        return selectedWeaponIndex >= 0;
    }
}
