#pragma once
#include "../enemy_base.hpp"

// Slime enemy class, inherits from EnemyBase
class SlimeEnemy : public EnemyBase
{
public:
    // Default constructor: fallback
    SlimeEnemy()
    {
        width = 64;  // Slime width
        height = 36; // Slime height
        hp = 100;    // Health points
        speed = 0.6; // Movement speed
    }

    // Constructor with wave parameter: used by spawn_enemies generator
    SlimeEnemy(int wave)
    {
        width = 64;  // Slime width
        height = 36; // Slime height
        hp = 100;    // Health points
        speed = 0.5; // Movement speed
    }

    void load_assets() override;                                             // Override to load assets
    void update(player_data &player, std::vector<Bullet> &bullets) override; // Override to update state (player, bullets)
    void draw() const override;                                              // Override to draw slime
    bool facing_left = false;                                                // Facing direction (false = right)

    // Right-facing frames (2 frames)
    bitmap right_frames[2];
    // Left-facing frames (2 frames)
    bitmap left_frames[2];

private:
    bitmap frames[2];         // General frame storage
    int frame_count{2};       // Total frame count
    int current_frame{0};     // Current active frame index
    int frame_timer{0};       // Timer for frame switching
    int frame_interval{20};   // Frames between frame updates
    int slow_timer = 0;       // Remaining frames of slow effect after hit
    double slow_factor = 0.5; // Speed reduction ratio (e.g., 0.5 = 50%)

protected:
    double speed{0.5}; // Base movement speed
};