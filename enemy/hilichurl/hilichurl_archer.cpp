// HilichurlArcher implementation: distance control, reload, slow arrows with spread.
#include "enemy/hilichurl/hilichurl_archer.hpp"
#include "player/player.hpp"
#include "coin.hpp"
#include <cmath>
#include <cstdlib>

extern player_data player; // from main.cpp

// Helpers for robust bullet collision (avoid tunneling)
static inline bool ha_point_in_rect(double px, double py, double rx, double ry, double rw, double rh)
{
    return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
}

static bool ha_segment_intersects_rect(double x1, double y1, double x2, double y2,
                                       double rx, double ry, double rw, double rh)
{
    if (ha_point_in_rect(x1, y1, rx, ry, rw, rh) || ha_point_in_rect(x2, y2, rx, ry, rw, rh))
        return true;

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
    if (seg_intersect(x1, y1, x2, y2, left, top, left, bottom)) return true;
    if (seg_intersect(x1, y1, x2, y2, right, top, right, bottom)) return true;
    if (seg_intersect(x1, y1, x2, y2, left, top, right, top)) return true;
    if (seg_intersect(x1, y1, x2, y2, left, bottom, right, bottom)) return true;
    return false;
}

void HilichurlArcher::load_assets()
{
    load_bitmap("h_arch_unloaded", "../image/enemy/hilichurl_archer/archer_unloaded.png");
    load_bitmap("h_arch_loaded", "../image/enemy/hilichurl_archer/archer_loaded.png");
    // Arrow sprite (change to Bullet_Alt2_2)
    load_bitmap("arrow_alt2_2", "../image/weapon/Bullet_Alt2_2.png");
    unloaded_ = bitmap_named("h_arch_unloaded");
    loaded_ = bitmap_named("h_arch_loaded");
    width = bitmap_width(unloaded_);
    height = bitmap_height(unloaded_);
    hp = 100;
    is_loaded_ = false;
    reload_timer_ = reload_time_;
}

void HilichurlArcher::update(player_data &player, std::vector<Bullet> &bullets)
{
    if (!alive) return;

    

    // Face player
    facing_left_ = (player.player_x > x);

    // Movement: keep distance
    double dx = player.player_x - x;
    double dy = player.player_y - y;
    double dist = sqrt(dx * dx + dy * dy);
    if (dist > 0)
    {
        dx /= dist; dy /= dist;
    }

    if (dist > prefer_dist_ + 20)
    {
        x += dx * speed_;
        y += dy * speed_;
    }
    else if (dist < min_dist_)
    {
        x -= dx * speed_;
        y -= dy * speed_;
    }
    // else hold position

    // Loading / firing
    if (!is_loaded_)
    {
        reload_timer_--;
        if (reload_timer_ <= 0)
        {
            is_loaded_ = true;
            loaded_display_timer_ = loaded_display_time_;
        }
    }
    else
    {
        if (loaded_display_timer_ > 0)
        {
            loaded_display_timer_--;
        }
        else
        {
            // Fire a slow projectile (reuse player's bullet sprite). Add inaccuracy.
            Arrow a; a.x = x + width/2; a.y = y + height/2; a.active = true;
            double spd = 3.5; // half speed
            double base_ang = atan2(dy, dx);
            double spread_deg = 24.0; // increased spread
            double rand_deg = (rand() % 1000 / 1000.0) * spread_deg * 2 - spread_deg;
            double ang = base_ang + rand_deg * (3.141592654 / 180.0);
            a.dx = cos(ang) * spd; a.dy = sin(ang) * spd;
            arrows_.push_back(a);

            // Reset reload
            is_loaded_ = false;
            reload_timer_ = reload_time_;
        }
    }

    // Update enemy arrows and check collision with player
    for (auto &a : arrows_)
    {
        if (!a.active) continue;
        a.x += a.dx;
        a.y += a.dy;
        if (a.x < -50 || a.x > screen_width()+50 || a.y < -50 || a.y > screen_height()+50)
            a.active = false;

        // Simple AABB vs player
        if (a.active && std::fabs((player.player_x + player.player_width/2) - a.x) < player.player_width/2 &&
            std::fabs((player.player_y + player.player_hight/2) - a.y) < player.player_hight/2)
        {
            if (player.blocking)
            {
                // successful block: small backstep only
                double back = 10.0;
                if (player.facing == FACING_LEFT) player.player_x += back; else player.player_x -= back;
            }
            else if (player.damage_cooldown <= 0)
            {
                player.hearts -= 1;
                if (player.hearts < 0) player.hearts = 0;
                player.damage_cooldown = player.damage_cooldown_max;
                player.just_got_hit = true;
                // Knockback along arrow direction
                double klen = sqrt(a.dx * a.dx + a.dy * a.dy);
                if (klen > 0)
                {
                    player.knockback_dx = (a.dx / klen) * 8;
                    player.knockback_dy = (a.dy / klen) * 8;
                    player.knockback_timer = 6;
                }
                if (player.hearts <= 0) player.alive = false;
            }
            a.active = false;
        }
    }

    // Coin / bullet interactions (player bullets hitting archer)
    // Damage by player's bullets (use full sprite rect + segment test)
    for (auto &b : bullets)
    {
        if (!b.active) continue;
        double rx = x, ry = y, rw = width, rh = height;
        double prev_x = b.x - b.dx;
        double prev_y = b.y - b.dy;
        bool hit = ha_point_in_rect(b.x, b.y, rx, ry, rw, rh) ||
                   ha_segment_intersects_rect(prev_x, prev_y, b.x, b.y, rx, ry, rw, rh);
        if (hit)
        {
            hp -= b.damage;
            if (!b.piercing) b.active = false;
            
            if (hp <= 0)
            {
                alive = false;
                extern int kill_marker_timer; kill_marker_timer = 12;
                extern std::vector<Coin> coins;
                Coin c; c.x = x + width / 2; c.y = y + height / 2; c.value = 4 + rand() % 3; c.active = true;
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
                c.frames = { bitmap_named("coin_1"), bitmap_named("coin_2"), bitmap_named("coin_3"), bitmap_named("coin_4"), bitmap_named("coin_5"), bitmap_named("coin_6"), bitmap_named("coin_7"), bitmap_named("coin_8"), bitmap_named("coin_9"), bitmap_named("coin_10") };
                c.frame = 0; c.frame_timer = 0; c.frame_interval = 20; c.frame_count = 10;
                coins.push_back(c);
            }
        }
    }

    if (player.damage_cooldown > 0) player.damage_cooldown--;
}

void HilichurlArcher::draw() const
{
    if (!alive) return;
    bitmap frame = is_loaded_ ? loaded_ : unloaded_;
    if (facing_left_)
    {
        drawing_options opts = option_flip_y();
        draw_bitmap(frame, x, y, opts);
    }
    else { draw_bitmap(frame, x, y); }

    

    // HP bar
    double bar_width = width;
    double hp_ratio = static_cast<double>(hp) / 100.0;
    fill_rectangle(COLOR_GREEN, x, y - 10, bar_width * hp_ratio, 5);
    draw_rectangle(COLOR_BLACK, x, y - 10, bar_width, 5);

    // Draw arrows using Bullet_Alt2_2
    bitmap arrow_img = bitmap_named("arrow_alt2_2");
    for (const auto &a : arrows_)
    {
        if (!a.active || !arrow_img) continue;
        double angle_deg = atan2(a.dy, a.dx) * 180.0 / 3.141592654;
        draw_bitmap(arrow_img, a.x, a.y, option_rotate_bmp(angle_deg));
    }
}
