package bulletbloom;

import java.util.Arrays;

/**
 * Tracks purchasable upgrades and unlocked weapon slots.
 */
public final class ShopState {
    /** Cost to heal one missing heart. */
    public static final int HEAL_COST = 15;

    /** Cost to add one max heart. */
    public static final int MAX_HEART_COST = 80;

    private static final int[] WEAPON_COSTS = {0, 120, 180, 260};

    private final boolean[] unlockedWeapons = new boolean[WEAPON_COSTS.length];

    /**
     * Creates the default shop state with only the starting weapon unlocked.
     */
    public ShopState() {
        unlockedWeapons[0] = true;
    }

    /**
     * Checks whether a weapon slot is unlocked.
     *
     * @param index weapon slot
     * @return {@code true} if the weapon is available
     */
    public boolean isWeaponUnlocked(int index) {
        return index >= 0 && index < unlockedWeapons.length && unlockedWeapons[index];
    }

    /**
     * Unlocks a weapon slot.
     *
     * @param index weapon slot
     */
    public void unlockWeapon(int index) {
        if (index >= 0 && index < unlockedWeapons.length) {
            unlockedWeapons[index] = true;
        }
    }

    /**
     * Gets a weapon unlock cost.
     *
     * @param index weapon slot
     * @return unlock cost, or {@code Integer.MAX_VALUE} for invalid slots
     */
    public int getWeaponCost(int index) {
        if (index < 0 || index >= WEAPON_COSTS.length) {
            return Integer.MAX_VALUE;
        }
        return WEAPON_COSTS[index];
    }

    /**
     * Gets a copy of the unlocked weapon flags.
     *
     * @return copied unlock flags
     */
    public boolean[] copyUnlockedWeapons() {
        return Arrays.copyOf(unlockedWeapons, unlockedWeapons.length);
    }

    /**
     * Restores weapon unlock flags from save data.
     *
     * @param unlocked saved unlock flags
     */
    public void restoreUnlockedWeapons(boolean[] unlocked) {
        Arrays.fill(unlockedWeapons, false);
        for (int index = 0; index < unlockedWeapons.length && index < unlocked.length; index++) {
            unlockedWeapons[index] = unlocked[index];
        }
        unlockedWeapons[0] = true;
    }

    /**
     * Resets shop progress for a new run.
     */
    public void reset() {
        Arrays.fill(unlockedWeapons, false);
        unlockedWeapons[0] = true;
    }
}
