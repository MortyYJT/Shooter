package bulletbloom.weapon;

import java.util.ArrayList;
import java.util.List;

/**
 * Creates the default migrated weapon set.
 */
public final class WeaponCatalog {
    private WeaponCatalog() {
        throw new IllegalStateException("Utility class");
    }

    /**
     * Creates the default weapon list used by a new run.
     *
     * @return ordered weapon slots
     */
    public static List<Weapon> createDefaultWeapons() {
        List<Weapon> loadedWeapons = new ArrayList<>();
        loadedWeapons.add(new Pistol());
        loadedWeapons.add(new GunWeapon(
                "AK-47",
                "/image/weapon/weapon_AK-47.png",
                "/image/weapon/bullet_0.png",
                20,
                100,
                70,
                true));
        loadedWeapons.add(new GunWeapon(
                "Shotgun",
                "/image/weapon/Shotgun.png",
                "/image/weapon/Bullet_fire.png",
                56,
                50,
                24,
                false,
                -10,
                -5,
                -2,
                0,
                2,
                5,
                10));
        loadedWeapons.add(new GunWeapon(
                "AWP",
                "/image/weapon/AWP.png",
                "/image/weapon/Bullet_Yellow.png",
                120,
                120,
                300,
                false));
        return loadedWeapons;
    }
}
