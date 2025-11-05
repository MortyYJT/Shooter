#pragma once
#include "splashkit.h"
#include "../player/player.hpp"
#include "../weapon/weapon_base.hpp"
#include <vector>

// Abstract base class for all enemies
class EnemyBase
{
public:
    double x{0}, y{0};          // X and Y coordinates
    double width{0}, height{0}; // Width and height
    int hp{0};                  // Health points
    bool alive{true};           // Alive status
    int slow_timer = 20;        // Slow effect timer
    int speed = 0.5;            // Movement speed
    int slow_factor = 0.5;      // Slow effect factor
    virtual ~EnemyBase() = default;

    // Load own assets for each enemy
    virtual void load_assets() = 0;

    // Update per frame (AI behavior, movement, taking damage)
    virtual void update(player_data &player, std::vector<Bullet> &bullets) = 0;

    // Draw itself
    virtual void draw() const = 0;
};
