#pragma once
#include "splashkit.h"

// Player facing direction
enum direction
{
    FACING_RIGHT,
    FACING_LEFT
};

struct player_data
{
    const int player_width = 48;  // Player width in pixels
    const int player_hight = 48;  // Player height in pixels
    double player_x;              // Player's x position
    double player_y;              // Player's y position
    double player_speed;          // Player movement speed
    int hearts;                   // Current remaining hearts
    int max_hearts;               // Max hearts (6)

    bool alive;                   // Whether the player is alive
    int damage_cooldown;          // Cooldown after taking damage
    int damage_cooldown_max;      // Max duration of damage cooldown

    bitmap frames[5];             // Animation frames
    int current_frame;            // Current animation frame index
    int frame_timer;              // Timer for frame switching
    int frame_interval;           // Interval between frame updates

    bitmap breath_frames[3];      // Breath animation frames
    int breath_frame;             // Current breath frame index
    int breath_timer;             // Timer for breath frame switching
    int breath_interval;          // Interval between breath frame updates

    direction facing;             // Current facing direction

    // Hit feedback system
    bool just_got_hit = false;    // Whether hit in current frame
    double knockback_dx = 0;      // Horizontal knockback force
    double knockback_dy = 0;      // Vertical knockback force
    int knockback_timer = 0;      // Knockback duration timer

    // Blocking system (Space to block)
    bool blocking = false;        // Currently blocking
    bitmap block_frames[4];       // Block animation frames
    int block_frame = 0;          // Current block frame index
    int block_timer = 0;          // Remaining block frames
    int block_interval = 5;       // Frames between block animation updates
    int block_duration = 20;      // Total block duration (frames)

    // Movement modifiers from weapons
    bool dash_disabled = false;   // If true, cannot dash
};

void draw_player(player_data &player);       // Draw the player
void update_player(player_data &player);     // Update player state
void load_player(player_data &player);       // Load player resources
void draw_player_hp(const player_data &player);  // Draw player's HP

// Blocking helpers (moved from main)
void update_player_block(player_data &player);            // Handle input and timers for blocking
void draw_player_block_overlay(const player_data &player); // Draw block animation overlay
