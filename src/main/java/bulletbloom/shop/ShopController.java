package bulletbloom.shop;

import bulletbloom.player.Player;
import bulletbloom.weapon.Weapon;
import java.util.List;

/**
 * Applies shop purchases and equipment changes.
 */
public final class ShopController {
    private final ShopState shopState;
    private final Wallet wallet;
    private final List<Weapon> weapons;

    /**
     * Creates a shop controller.
     *
     * @param shopState mutable shop state
     * @param wallet mutable wallet
     * @param weapons available weapon slots
     */
    public ShopController(ShopState shopState, Wallet wallet, List<Weapon> weapons) {
        this.shopState = shopState;
        this.wallet = wallet;
        this.weapons = weapons;
    }

    /**
     * Attempts to buy one heart of healing.
     *
     * @param player current player
     * @return purchase result
     */
    public ShopResult buyHeal(Player player) {
        if (player.getHearts() >= player.getMaxHearts()) {
            return ShopResult.message("HP full");
        }
        if (!wallet.spend(ShopState.HEAL_COST)) {
            return ShopResult.message("Not enough money");
        }
        player.healOneHeart();
        return ShopResult.message("Healed");
    }

    /**
     * Attempts to buy one maximum heart upgrade.
     *
     * @param player current player
     * @return purchase result
     */
    public ShopResult buyMaxHeart(Player player) {
        if (!wallet.spend(ShopState.MAX_HEART_COST)) {
            return ShopResult.message("Not enough money");
        }
        player.increaseMaxHearts();
        return ShopResult.message("Max HP increased");
    }

    /**
     * Attempts to buy or equip a weapon.
     *
     * @param weaponIndex requested weapon slot
     * @return purchase or equip result
     */
    public ShopResult buyOrEquipWeapon(int weaponIndex) {
        if (weaponIndex < 0 || weaponIndex >= weapons.size()) {
            return ShopResult.message("Invalid weapon");
        }
        if (shopState.isWeaponUnlocked(weaponIndex)) {
            return ShopResult.equipped(weaponIndex, "Equipped " + weapons.get(weaponIndex).getName());
        }
        int cost = shopState.getWeaponCost(weaponIndex);
        if (!wallet.spend(cost)) {
            return ShopResult.message("Not enough money");
        }
        shopState.unlockWeapon(weaponIndex);
        return ShopResult.equipped(weaponIndex, "Unlocked " + weapons.get(weaponIndex).getName());
    }
}
