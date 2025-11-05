#include "splashkit.h"
#include "player.hpp"

// Load player image resources
void load_player(player_data &player)
{
    // Load player walking animation frames
    load_bitmap("Lumine_walking_0", "../image/player/player_walking/Lumine_walking_0.png");
    load_bitmap("Lumine_walking_1", "../image/player/player_walking/Lumine_walking_1.png");
    load_bitmap("Lumine_walking_2", "../image/player/player_walking/Lumine_walking_2.png");
    load_bitmap("Lumine_walking_3", "../image/player/player_walking/Lumine_walking_3.png");
    load_bitmap("Lumine_walking_4", "../image/player/player_walking/Lumine_walking_4.png");

    // Load player idle breathing animation frames
    load_bitmap("Lumine_breath_0", "../image/player/player_breath/Lumine_breath_0.png");
    load_bitmap("Lumine_breath_1", "../image/player/player_breath/Lumine_breath_1.png");
    load_bitmap("Lumine_breath_2", "../image/player/player_breath/Lumine_breath_2.png");

    // Store breathing animation frames in array
    player.breath_frames[0] = bitmap_named("Lumine_breath_0");
    player.breath_frames[1] = bitmap_named("Lumine_breath_1");
    player.breath_frames[2] = bitmap_named("Lumine_breath_2");

    // Init breathing animation parameters
    player.breath_frame = 0;       // Current breathing frame index
    player.breath_timer = 0;       // Timer for breathing frame switch
    player.breath_interval = 60;   // Frame interval for breathing animation switch (60 frames)

    // Store walking animation frames in array
    player.frames[0] = bitmap_named("Lumine_walking_0");
    player.frames[1] = bitmap_named("Lumine_walking_1");
    player.frames[2] = bitmap_named("Lumine_walking_2");
    player.frames[3] = bitmap_named("Lumine_walking_3");
    player.frames[4] = bitmap_named("Lumine_walking_4");

    // Init walking animation parameters
    player.current_frame = 0;      // Current walking frame index
    player.frame_timer = 0;        // Timer for walking frame switch
    player.frame_interval = 8;     // Frame interval for walking animation switch (8 frames)

    // Set default direction and health
    player.facing = FACING_LEFT;   // Initial facing direction
    player.hearts = 6;             // Current health (hearts)
    player.max_hearts = 6;         // Maximum health (hearts)

    player.alive = true;           // Player alive status

    // Init damage cooldown (prevent continuous damage)
    player.damage_cooldown = 0;        // Current damage cooldown timer
    player.damage_cooldown_max = 240;  // Max damage cooldown (~4 seconds)
    // Preload UI hearts once during player setup
    load_bitmap("heart_full", "../image/ui/heart_full.png");
    load_bitmap("heart_empty", "../image/ui/heart_empty.png");

    // Load block overlay frames and defaults
    load_bitmap("player_block_0", "../image/player/player_block/player_block_0.png");
    load_bitmap("player_block_1", "../image/player/player_block/player_block_1.png");
    load_bitmap("player_block_2", "../image/player/player_block/player_block_2.png");
    load_bitmap("player_block_3", "../image/player/player_block/player_block_3.png");
    player.block_frames[0] = bitmap_named("player_block_0");
    player.block_frames[1] = bitmap_named("player_block_1");
    player.block_frames[2] = bitmap_named("player_block_2");
    player.block_frames[3] = bitmap_named("player_block_3");
    player.block_duration = 20;
    player.block_interval = 5;
}

// Draw current player frame
void draw_player(player_data &player)
{
    bitmap frame;

    // Use walking animation if moving, else breathing animation
    if (key_down(A_KEY) || key_down(D_KEY) || key_down(W_KEY) || key_down(S_KEY))
    {
        frame = player.frames[player.current_frame];
    }
    else
    {
        frame = player.breath_frames[player.breath_frame];
    }

    // Draw based on facing direction (flip for left)
    if (player.facing == FACING_LEFT)
    {
        drawing_options opts = option_flip_y();
        draw_bitmap(frame, player.player_x, player.player_y, opts);
    }
    else
    {
        draw_bitmap(frame, player.player_x, player.player_y);
    }
}

// Global variables for dash system
bool is_dashing = false;      // Dash status flag
int dash_timer = 0;           // Dash duration timer (frames)
double dash_speed = 0;        // Dash movement speed
double dash_dir_x = 0;        // Dash X direction (-1, 0, 1)
double dash_dir_y = 0;        // Dash Y direction (-1, 0, 1)

// Update player state (movement/dash/animation)
void update_player(player_data &player)
{
    bool moving = false;
    // Knockback handling
    if (player.knockback_timer > 0)
    {
        player.player_x += player.knockback_dx;
        player.player_y += player.knockback_dy;

        // Knockback attenuation
        player.knockback_dx *= 0.8;
        player.knockback_dy *= 0.8;

        player.knockback_timer--;
        return; // Can't control movement during knockback
    }

    // Trigger dash on Left Shift press (if not dashing and moving)
    if (key_typed(LEFT_SHIFT_KEY) && !is_dashing && !player.dash_disabled &&
        (key_down(W_KEY) || key_down(A_KEY) || key_down(S_KEY) || key_down(D_KEY)))
    {
        is_dashing = true;
        dash_timer = 10; // Dash duration (10 frames)
        dash_speed = 20; // Dash speed value
        dash_dir_x = 0;
        dash_dir_y = 0;

        // Determine dash direction from key presses
        if (key_down(W_KEY))
            dash_dir_y = -1;
        if (key_down(S_KEY))
            dash_dir_y = 1;
        if (key_down(A_KEY))
            dash_dir_x = -1;
        if (key_down(D_KEY))
            dash_dir_x = 1;

        // Normalize direction vector (prevent faster diagonal movement)
        double len = sqrt(dash_dir_x * dash_dir_x + dash_dir_y * dash_dir_y);
        if (len > 0)
        {
            dash_dir_x /= len;
            dash_dir_y /= len;
        }
    }

    // Dash state
    if (is_dashing)
    {
        player.player_x += dash_dir_x * dash_speed;
        player.player_y += dash_dir_y * dash_speed;
        dash_timer--;

        // End dash when timer ends
        if (dash_timer <= 0)
        {
            is_dashing = false;
        }
    }
    else // Normal movement
    {
        if (key_down(A_KEY))
            player.player_x -= player.player_speed,
                player.facing = FACING_LEFT,
                moving = true;
        if (key_down(D_KEY))
            player.player_x += player.player_speed,
                player.facing = FACING_RIGHT,
                moving = true;
        if (key_down(W_KEY))
            player.player_y -= player.player_speed,
                moving = true;
        if (key_down(S_KEY))
            player.player_y += player.player_speed,
                moving = true;
    }

    // Update walking animation
    if (moving)
    {
        player.frame_timer++;
        if (player.frame_timer >= player.frame_interval)
        {
            player.current_frame = (player.current_frame + 1) % 5;
            player.frame_timer = 0;
        }
    }
    else
    {
        player.current_frame = 0;
        player.frame_timer = 0;
    }

    // Update breathing animation
    if (!moving)
    {
        player.breath_timer++;
        if (player.breath_timer >= player.breath_interval)
        {
            player.breath_frame = (player.breath_frame + 1) % 3;
            player.breath_timer = 0;
        }
    }
    else
    {
        player.breath_timer = 0;
        player.breath_frame = 0;
    }

    // Screen boundary constraints
    if (player.player_x < 0)
        player.player_x = 0;
    if (player.player_y < 0)
        player.player_y = 0;
    if (player.player_x > 1600 - 48)  // 1600: screen width; 48: player width
        player.player_x = 1600 - 48;
    if (player.player_y > 1200 - 48)  // 1200: screen height; 48: player height
        player.player_y = 1200 - 48;
}

// Draw player health bar
void draw_player_hp(const player_data &player)
{
    bitmap heart_full = bitmap_named("heart_full");
    bitmap heart_empty = bitmap_named("heart_empty");

    double start_x = 20;
    double start_y = 20;
    double gap = 0;
    double size = 48;
    int per_row = 6; // 6 per row, then wrap
    for (int i = 0; i < player.max_hearts; i++)
    {
        int row = i / per_row;
        int col = i % per_row;
        double x = start_x + col * (size + gap);
        double y = start_y + row * (size + gap);
        if (i < player.hearts) draw_bitmap(heart_full, x, y);
        else draw_bitmap(heart_empty, x, y);
    }
}

// === Blocking logic moved from main ===
void update_player_block(player_data &player)
{
    if (key_typed(SPACE_KEY) && !player.blocking && player.knockback_timer <= 0)
    {
        player.blocking = true;
        player.block_timer = player.block_duration;
        player.block_frame = 0;
    }

    if (player.blocking)
    {
        player.block_timer--;
        if (player.block_timer <= 0)
        {
            player.blocking = false;
            player.block_frame = 0;
        }
        else if ((player.block_duration - player.block_timer) % player.block_interval == 0)
        {
            player.block_frame = (player.block_frame + 1) % 4;
        }
    }
}

void draw_player_block_overlay(const player_data &player)
{
    if (!player.blocking) return;
    bitmap bframe = player.block_frames[player.block_frame];
    if (!bframe) return;
    if (player.facing == FACING_LEFT)
    {
        drawing_options opts = option_flip_y();
        draw_bitmap(bframe, player.player_x, player.player_y, opts);
    }
    else
    {
        draw_bitmap(bframe, player.player_x, player.player_y);
    }
}
