#pragma once
#include "splashkit.h"
#include "../player/player.hpp"
#include <vector>
#include <cmath>

// Bullet structure representing each fired projectile
struct Bullet
{
    double x = 0; // Bullet position
    double y = 0;
    double dx = 0; // Bullet direction vector
    double dy = 0;
    double speed = 0;        // Bullet speed
    bool active = false;     // Whether bullet is active
    bool was_active = false; // Previous frame state for collision detection
    int weapon_id = -1;      // ID of the weapon that fired the bullet
    int damage = 0;          // Damage value of the bullet
    bitmap image;            // Bullet sprite
    bool piercing = false;   // If true, bullet does not deactivate on hit
};

// Spark structure for bullet hit visual effect
struct Spark
{
    double x, y;   // Spark position
    double dx, dy; // Spark velocity
    double life;   // Spark line length
    double length; // Spark lifetime
};

extern std::vector<Spark> sparks;

// Abstract base class for all weapons
class WeaponBase
{
public:
    virtual ~WeaponBase() = default;                    // Virtual destructor
    virtual void load_assets() = 0;                     // Load textures and resources
    virtual void update(const player_data &player) = 0; // Update weapon logic
    virtual void draw(const player_data &player) = 0;   // Draw weapon and bullets
    virtual std::vector<Bullet> &bullets() = 0;         // Return reference to bullet list
};

// Pistol class derived from WeaponBase
class Pistol : public WeaponBase
{
public:
    void load_assets() override;                                 // Load pistol assets
    void update(const player_data &player) override;             // Update pistol logic
    void draw(const player_data &player) override;               // Draw pistol and bullets
    std::vector<Bullet> &bullets() override { return bullets_; } // Access bullet list

private:
    bitmap image_{nullptr};       // Pistol sprite
    std::vector<Bullet> bullets_; // Bullets fired by pistol

    int fire_cooldown = 0;        // Fire cooldown timer
    const int fire_interval = 20; // Frames between shots

    bool is_recoiling = false;          // Whether pistol is recoiling
    int recoil_timer = 0;               // Recoil timer
    const int recoil_duration = 10;     // Total recoil duration
    const double recoil_strength = 5.0; // Strength of recoil effect
};

// AK class derived from WeaponBase
class AK : public WeaponBase
{
public:
    void load_assets() override;                                 // Load AK assets
    void update(const player_data &player) override;             // Update AK logic
    void draw(const player_data &player) override;               // Draw AK and bullets
    std::vector<Bullet> &bullets() override { return bullets_; } // Access bullet list

private:
    bitmap image_{nullptr};       // AK sprite
    std::vector<Bullet> bullets_; // Bullets fired by AK

    int fire_cooldown_ = 0;        // Fire cooldown timer
    const int fire_interval_ = 20; // Frames between shots

    bool is_recoiling_ = false;          // Whether AK is recoiling
    int recoil_timer_ = 0;               // Recoil timer
    const int recoil_duration_ = 20;     // Total recoil duration
    const double recoil_strength_ = 8.0; // Strength of recoil effect
};

// Shotgun weapon
class Shotgun : public WeaponBase
{
public:
    void load_assets() override;
    void update(const player_data &player) override;
    void draw(const player_data &player) override;
    std::vector<Bullet> &bullets() override { return bullets_; }

private:
    bitmap image_{nullptr};
    std::vector<Bullet> bullets_;
    int fire_cooldown_ = 0;
    const int fire_interval_ = 56; // reduced rate (half of previous)
};

// AWP sniper (piercing, high damage, slow)
class AWP : public WeaponBase
{
public:
    void load_assets() override;
    void update(const player_data &player) override;
    void draw(const player_data &player) override;
    std::vector<Bullet> &bullets() override { return bullets_; }

private:
    bitmap image_{nullptr};
    std::vector<Bullet> bullets_;
    int fire_cooldown_ = 0;
    const int fire_interval_ = 120; // slower rate to match SFX length
};

// Gatling (high ROF, spread). While equipped: slow move, dash disabled.
// (Gatling and Kelber removed)
