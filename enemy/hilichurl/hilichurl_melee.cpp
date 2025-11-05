// HilichurlMelee implementation: telegraph -> attack -> recover; supports blocking.
#include "enemy/hilichurl/hilichurl_melee.hpp"
#include "player/player.hpp"
#include "weapon/weapon_base.hpp"
#include "coin.hpp"
#include <cmath>
#include <cstdlib>

extern player_data player; // from main.cpp

// Helpers for robust bullet collision (avoid tunneling)
static inline bool h_point_in_rect(double px, double py, double rx, double ry, double rw, double rh)
{
    return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
}

static bool h_segment_intersects_rect(double x1, double y1, double x2, double y2,
                                      double rx, double ry, double rw, double rh)
{
    if (h_point_in_rect(x1, y1, rx, ry, rw, rh) || h_point_in_rect(x2, y2, rx, ry, rw, rh))
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

void HilichurlMelee::load_assets()
{
    load_bitmap("h_melee_idle", "../image/enemy/hilichurl/melee_idle.png");
    load_bitmap("h_melee_a0", "../image/enemy/hilichurl/melee_attack_0.png");
    load_bitmap("h_melee_a1", "../image/enemy/hilichurl/melee_attack_1.png");
    load_bitmap("h_melee_a2", "../image/enemy/hilichurl/melee_attack_2.png");

    idle_ = bitmap_named("h_melee_idle");
    attack_frames_[0] = bitmap_named("h_melee_a0");
    attack_frames_[1] = bitmap_named("h_melee_a1");
    attack_frames_[2] = bitmap_named("h_melee_a2");

    width = bitmap_width(idle_);
    height = bitmap_height(idle_);
    hp = 120;
}

void HilichurlMelee::update(player_data &player, std::vector<Bullet> &bullets)
{
    if (!alive) return;

    

    // Face player
    facing_left_ = (player.player_x > x);

    // Move towards player when chasing
    if (state_ == CHASE)
    {
        double dx = player.player_x - x;
        double dy = player.player_y - y;
        double len = sqrt(dx * dx + dy * dy);
        if (len > 0)
        {
            dx /= len; dy /= len;
            x += dx * speed_;
            y += dy * speed_;
        }

        // If touching player, start telegraph instead of damage
        double pdx = std::fabs(player.player_x - x);
        double pdy = std::fabs(player.player_y - y);
        if (pdx < (player.player_width + width / 2) && pdy < (player.player_hight + height / 2))
        {
            state_ = TELEGRAPH;
            telegraph_timer_ = telegraph_duration_;
            // Play telegraph sound
            load_sound_effect("attack_sfx", "../sound/attack.mp3");
            play_sound_effect("attack_sfx", 0.6);
        }
    }
    else if (state_ == TELEGRAPH)
    {
        telegraph_timer_--;
        // Extra-lenient parry: blocking during telegraph also counts
        if (player.blocking && !was_blocked_)
        {
            // Backstep and mark as blocked to avoid damage later
            double back = 10.0;
            if (player.facing == FACING_LEFT) player.player_x += back; else player.player_x -= back;
            load_sound_effect("block_sfx", "../sound/block.mp3");
            play_sound_effect("block_sfx", 0.7);
            was_blocked_ = true;
            dealt_damage_ = true;
        }
        if (telegraph_timer_ <= 0)
        {
            state_ = ATTACK;
            atk_frame_ = 0; atk_timer_ = 0; dealt_damage_ = false;
            // If already blocked during telegraph, keep the blocked flag (dealt_damage_ will be reset above; restore)
            if (was_blocked_) dealt_damage_ = true;
        }
    }
    else if (state_ == ATTACK)
    {
        atk_timer_++;
        if (atk_timer_ >= atk_interval_)
        {
            atk_timer_ = 0;
            atk_frame_++;
        }

        // If player is blocking at any point during the attack window, count as successful block (lenient)
        if (!dealt_damage_ && player.blocking)
        {
            // Backstep only on successful block
            double back = 10.0;
            if (player.facing == FACING_LEFT) player.player_x += back; else player.player_x -= back;
            // Play block sound
            load_sound_effect("block_sfx", "../sound/block.mp3");
            play_sound_effect("block_sfx", 0.7);
            dealt_damage_ = true; // consume this attack
            was_blocked_ = true;
        }

        // Deal damage around the 2nd frame (core hit frame)
        if (atk_frame_ == 1 && !dealt_damage_)
        {
            double dx = std::fabs(player.player_x - x);
            double dy = std::fabs(player.player_y - y);
            if (dx < (player.player_width + width / 2) && dy < (player.player_hight + height / 2))
            {
                if (!(player.blocking))
                {
                    if (player.damage_cooldown <= 0)
                    {
                        player.hearts -= 1;
                        if (player.hearts < 0) player.hearts = 0;
                        player.damage_cooldown = player.damage_cooldown_max;
                        player.just_got_hit = true;

                        // Knockback
                        double kx = player.player_x - x;
                        double ky = player.player_y - y;
                        double klen = sqrt(kx * kx + ky * ky);
                        if (klen > 0)
                        {
                            kx /= klen; ky /= klen;
                            player.knockback_dx = kx * 10;
                            player.knockback_dy = ky * 10;
                            player.knockback_timer = 8;
                        }
                        if (player.hearts <= 0) player.alive = false;
                    }
                }
                else
                {
                    // Blocking during the hit frame: successful block backstep
                    double back = 10.0;
                    if (player.facing == FACING_LEFT) player.player_x += back; else player.player_x -= back;
                    load_sound_effect("block_sfx", "../sound/block.mp3");
                    play_sound_effect("block_sfx", 0.7);
                    was_blocked_ = true;
                }
                // prevent duplicate processing
                dealt_damage_ = true;
            }
        }

        if (atk_frame_ >= 3)
        {
            state_ = RECOVER;
            recover_timer_ = was_blocked_ ? blocked_recover_duration_ : recover_duration_;
            was_blocked_ = false;
        }
    }
    else if (state_ == RECOVER)
    {
        recover_timer_--;
        if (recover_timer_ <= 0)
        {
            state_ = CHASE;
        }
    }

    // Bullet collisions (use full sprite rect + segment test)
    for (auto &b : bullets)
    {
        if (!b.active) continue;
        double rx = x, ry = y, rw = width, rh = height;
        double prev_x = b.x - b.dx;
        double prev_y = b.y - b.dy;
        bool hit = h_point_in_rect(b.x, b.y, rx, ry, rw, rh) ||
                   h_segment_intersects_rect(prev_x, prev_y, b.x, b.y, rx, ry, rw, rh);
        if (hit)
        {
            hp -= b.damage;
            if (!b.piercing) b.active = false;
            
            if (hp <= 0)
            {
                alive = false;
                extern int kill_marker_timer; kill_marker_timer = 12;
                // spawn coins
                extern std::vector<Coin> coins;
                Coin c; c.x = x + width / 2; c.y = y + height / 2; c.value = 3 + rand() % 3; c.active = true;
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

    // Cooldown decrement (follow existing pattern)
    if (player.damage_cooldown > 0) player.damage_cooldown--;
}

void HilichurlMelee::draw() const
{
    if (!alive) return;

    bitmap frame = idle_;
    if (state_ == ATTACK)
    {
        int idx = atk_frame_;
        if (idx < 0) idx = 0; if (idx > 2) idx = 2;
        frame = attack_frames_[idx];
    }

    if (facing_left_)
    {
        drawing_options opts = option_flip_y();
        draw_bitmap(frame, x, y, opts);
    }
    else
    {
        draw_bitmap(frame, x, y);
    }

    // Hit white flash overlay
    

    // Telegraph visual: long yellow cross to screen edges
    if (state_ == TELEGRAPH)
    {
        double cx = x + width / 2;
        double cy = y + height / 2;
        color tele = COLOR_YELLOW;
        // Horizontal line across screen
        draw_line(tele, 0, cy, screen_width(), cy);
        draw_line(tele, 0, cy+1, screen_width(), cy+1); // slight thickness
        // Vertical line across screen
        draw_line(tele, cx, 0, cx, screen_height());
        draw_line(tele, cx+1, 0, cx+1, screen_height());
    }

    // HP bar
    double bar_width = width;
    double hp_ratio = static_cast<double>(hp) / 120.0;
    fill_rectangle(COLOR_GREEN, x, y - 10, bar_width * hp_ratio, 5);
    draw_rectangle(COLOR_BLACK, x, y - 10, bar_width, 5);
}
