// Slime enemy: simple chaser with 2-frame animation; slows when hit; drops coins.
#include "enemy/slime/slime.hpp"
#include "weapon/weapon_base.hpp"
#include "player/player.hpp"
#include "coin.hpp"
#include <cmath>
#include <cstdlib>

extern player_data player; // Global player object from main.cpp

// Helper: point inside rect
static inline bool point_in_rect(double px, double py, double rx, double ry, double rw, double rh)
{
    return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
}

// Helper: segment-rectangle intersection (handles fast bullets tunneling through)
static bool segment_intersects_rect(double x1, double y1, double x2, double y2,
                                    double rx, double ry, double rw, double rh)
{
    // Trivial accept: endpoints inside
    if (point_in_rect(x1, y1, rx, ry, rw, rh) || point_in_rect(x2, y2, rx, ry, rw, rh))
        return true;

    // Define rectangle edges as segments and test for intersection
    auto seg_intersect = [](double ax, double ay, double bx, double by,
                            double cx, double cy, double dx, double dy) -> bool {
        auto cross = [](double ux, double uy, double vx, double vy) { return ux * vy - uy * vx; };
        double abx = bx - ax, aby = by - ay;
        double acx = cx - ax, acy = cy - ay;
        double adx = dx - ax, ady = dy - ay;
        double cdx = dx - cx, cdy = dy - cy;
        double cax = ax - cx, cay = ay - cy;

        double d1 = cross(abx, aby, acx, acy);
        double d2 = cross(abx, aby, adx, ady);
        double d3 = cross(cdx, cdy, cax, cay);
        double d4 = cross(cdx, cdy, bx - cx, by - cy);

        if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
            ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0)))
            return true;

        return false;
    };

    double left = rx, right = rx + rw, top = ry, bottom = ry + rh;
    // Left edge
    if (seg_intersect(x1, y1, x2, y2, left, top, left, bottom)) return true;
    // Right edge
    if (seg_intersect(x1, y1, x2, y2, right, top, right, bottom)) return true;
    // Top edge
    if (seg_intersect(x1, y1, x2, y2, left, top, right, top)) return true;
    // Bottom edge
    if (seg_intersect(x1, y1, x2, y2, left, bottom, right, bottom)) return true;

    return false;
}

// =======================
// Load slime textures
// =======================
void SlimeEnemy::load_assets()
{
    // ---------- Randomly select slime color (0-3) ----------
    int type = rand() % 4;
    std::string prefix;

    switch (type)
    {
    case 0:
        prefix = "slime_yellow";
        break;
    case 1:
        prefix = "slime_purple";
        break;
    case 2:
        prefix = "slime_red";
        break;
    case 3:
        prefix = "slime_blue";
        break;
    }

    // ---------- Load 4 textures ----------
    // Naming rule:
    // slime_{color}_0.png → right-facing frame 1
    // slime_{color}_1.png → right-facing frame 2
    // slime_{color}_0_1.png → left-facing frame 1
    // slime_{color}_1_1.png → left-facing frame 2
    load_bitmap(prefix + "_r0", "../image/enemy/slime/" + prefix + "_0.png");
    load_bitmap(prefix + "_r1", "../image/enemy/slime/" + prefix + "_1.png");
    load_bitmap(prefix + "_l0", "../image/enemy/slime/" + prefix + "_0_1.png");
    load_bitmap(prefix + "_l1", "../image/enemy/slime/" + prefix + "_1_1.png");

    // Store in animation frame arrays
    right_frames[0] = bitmap_named(prefix + "_r0");
    right_frames[1] = bitmap_named(prefix + "_r1");
    left_frames[0] = bitmap_named(prefix + "_l0");
    left_frames[1] = bitmap_named(prefix + "_l1");

    // Common animation params
    frame_count = 2;     // Total animation frames
    current_frame = 0;   // Current frame index
    frame_timer = 0;     // Timer for frame switch
    frame_interval = 30; // Frames between frame switches (adjust speed)
    // Set collision rect size
    width = bitmap_width(right_frames[0]);   // Collision rect width
    height = bitmap_height(right_frames[0]); // Collision rect height

    facing_left = false; // Facing direction flag
}

// =======================
// Update slime logic
// =======================
void SlimeEnemy::update(player_data &player, std::vector<Bullet> &bullets)
{
    if (!alive)
        return;

    

    // ---------- Animation timing ----------
    frame_timer++;
    if (frame_timer >= frame_interval)
    {
        current_frame = (current_frame + 1) % frame_count;
        frame_timer = 0;
    }

    // ---------- Move towards player ----------
    double dir_x = player.player_x - x;               // X direction to player
    double dir_y = player.player_y - y;               // Y direction to player
    double len = sqrt(dir_x * dir_x + dir_y * dir_y); // Distance to player

    if (len > 0)
    {
        dir_x /= len; // Normalize X direction
        dir_y /= len; // Normalize Y direction

        double actual_speed = speed; // Base movement speed
        if (slow_timer > 0)          // If slowed
        {
            actual_speed *= slow_factor; // Apply slow multiplier
            slow_timer--;                // Decrease slow duration
        }

        x += dir_x * actual_speed; // Update X position
        y += dir_y * actual_speed; // Update Y position
    }

    // ---------- Determine facing direction ----------
    facing_left = (player.player_x > x); // True if player is to the right

    // ---------- Attack player (use reduced slime hit area: bottom half) ----------
    double px = player.player_x;
    double py = player.player_y;
    double pw = player.player_width;
    double ph = player.player_hight;
    double sx = x;
    double sy = y + height * 0.55; // only lower ~45% counts for damaging player
    double sw = width;
    double sh = height - height * 0.55;
    bool player_hit = (px < sx + sw && px + pw > sx && py < sy + sh && py + ph > sy);
    if (player_hit) // Collision check with reduced area
    {
        if (player.damage_cooldown <= 0) // If player not invulnerable
        {
            player.hearts -= 1; // Reduce player health
            if (player.hearts < 0)
                player.hearts = 0;
            player.damage_cooldown = player.damage_cooldown_max; // Reset invulnerability timer
            player.just_got_hit = true;                          // Flag for hit feedback

            // Knockback effect
            double kx = player.player_x - x;       // X knockback direction
            double ky = player.player_y - y;       // Y knockback direction
            double klen = sqrt(kx * kx + ky * ky); // Knockback direction length
            if (klen > 0)
            {
                kx /= klen;                    // Normalize X knockback
                ky /= klen;                    // Normalize Y knockback
                player.knockback_dx = kx * 10; // X knockback force
                player.knockback_dy = ky * 10; // Y knockback force
                player.knockback_timer = 8;    // Knockback duration
            }

            if (player.hearts <= 0)
                player.alive = false; // Player death
        }
    }

    if (player.damage_cooldown > 0)
        player.damage_cooldown--; // Decrease invulnerability timer

    // ---------- Bullet hit detection (full sprite rect + segment test) ----------
    for (auto &b : bullets)
    {
        if (!b.active) // Skip inactive bullets
            continue;

        double rx = x, ry = y, rw = width, rh = height;
        double prev_x = b.x - b.dx;
        double prev_y = b.y - b.dy;
        bool hit = point_in_rect(b.x, b.y, rx, ry, rw, rh) ||
                    segment_intersects_rect(prev_x, prev_y, b.x, b.y, rx, ry, rw, rh);
        if (hit)
        {
            hp -= b.damage;   // Reduce slime health
            if (!b.piercing) b.active = false; // Deactivate bullet unless piercing
            slow_timer = 60;  // Set slow duration
            

            // ---------- Slime death ----------
            if (hp <= 0)
            {
                alive = false; // Mark as dead
                extern int kill_marker_timer; // Show kill hitmarker near crosshair
                kill_marker_timer = 12;

                // ✅ Spawn coin animation
                extern std::vector<Coin> coins;
                Coin c;
                c.x = x + width / 2;      // Coin X position
                c.y = y + height / 2;     // Coin Y position
                c.value = 2 + rand() % 3; // Coin value (2-4)
                c.active = true;          // Activate coin

                // Load coin animation frames
                load_bitmap("coin_1", "../image/ui/coin1.png");
                load_bitmap("coin_2", "../image/ui/coin2.png");
                load_bitmap("coin_3", "../image/ui/coin3.png");
                load_bitmap("coin_4", "../image/ui/coin4.png");
                load_bitmap("coin_5", "../image/ui/coin5.png");
                load_bitmap("coin_6", "../image/ui/coin6.png");
                load_bitmap("coin_7", "../image/ui/coin7.png");
                load_bitmap("coin_8", "../image/ui/coin8.png");
                load_bitmap("coin_9", "../image/ui/coin9.png");
                load_bitmap("coin_10", "../image/ui/coin10.png");

                c.frames = {
                    bitmap_named("coin_1"), bitmap_named("coin_2"), bitmap_named("coin_3"),
                    bitmap_named("coin_4"), bitmap_named("coin_5"), bitmap_named("coin_6"),
                    bitmap_named("coin_7"), bitmap_named("coin_8"), bitmap_named("coin_9"),
                    bitmap_named("coin_10")};
                c.frame = 0;           // Current coin frame
                c.frame_timer = 0;     // Coin frame timer
                c.frame_interval = 20; // Coin frame switch interval
                c.frame_count = 10;    // Total coin frames

                coins.push_back(c); // Add coin to list
            }
        }
    }
}

// =======================
// Draw slime
// =======================
void SlimeEnemy::draw() const
{
    if (!alive)
        return;

    // Select frame based on facing direction
    bitmap frame;
    if (facing_left)
        frame = left_frames[current_frame];
    else
        frame = right_frames[current_frame];

    draw_bitmap(frame, x, y);

    

    // ---------- Draw health bar ----------
    double bar_width = width;                                        // Health bar width
    double hp_ratio = static_cast<double>(hp) / 100.0;               // Health percentage (0-1)
    fill_rectangle(COLOR_GREEN, x, y - 10, bar_width * hp_ratio, 5); // Green fill (current health)
    draw_rectangle(COLOR_BLACK, x, y - 10, bar_width, 5);            // Black border
}
