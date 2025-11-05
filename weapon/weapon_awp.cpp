// AWP sniper rifle: high damage, very slow fire rate.
// Now has muzzle flash, shell ejection, non-piercing, AK-like recoil, sparks on bullet destroy.
#include "weapon_base.hpp"
#include <cmath>
#include <vector>

static double AWP_PI = 3.141592654;

// Local VFX state (similar to AK but isolated)
static std::vector<Spark> awp_sparks;
struct AWP_Shell { double x,y,dx,dy,rotation,spin,life; bitmap image; };
static std::vector<AWP_Shell> awp_shells;
static bitmap awp_shell_img;
static bitmap awp_muzzle_flash_img;
static int awp_muzzle_timer = 0;
static double awp_muzzle_x = 0, awp_muzzle_y = 0;

// Recoil state (AK-like)
static bool awp_is_recoiling = false;
static int awp_recoil_timer = 0;
static const int awp_recoil_duration = 20;
static const double awp_recoil_strength = 8.0;

static void awp_create_sparks(double x, double y, double ang_rad)
{
    int count = 6 + rand() % 4;
    for (int i = 0; i < count; ++i)
    {
        Spark s; s.x = x; s.y = y;
        double spread = ((rand() % 41) - 20) * (AWP_PI / 180.0);
        double dir = ang_rad + AWP_PI + spread;
        double spd = 6 + rand() % 6;
        s.dx = cos(dir) * spd; s.dy = sin(dir) * spd;
        s.length = 10 + rand() % 14; s.life = 1.0;
        awp_sparks.push_back(s);
    }
}

static void awp_update_bullets(std::vector<Bullet> &arr)
{
    for (auto &b : arr)
    {
        bool prev = b.was_active; b.was_active = b.active;
        if (prev && !b.active)
        {
            awp_create_sparks(b.x, b.y, atan2(b.dy, b.dx));
            continue;
        }
        if (!b.active) continue;
        b.x += b.dx; b.y += b.dy;
        if (b.x < 0 || b.x > screen_width() || b.y < 0 || b.y > screen_height()) b.active = false;
    }
    for (auto &s : awp_sparks) { s.x += s.dx; s.y += s.dy; s.life -= 0.05; }
    std::vector<Spark> alive; alive.reserve(awp_sparks.size());
    for (auto &s : awp_sparks) if (s.life > 0) alive.push_back(s); awp_sparks = std::move(alive);
    for (auto &s : awp_shells) { s.x += s.dx; s.y += s.dy; s.dy += 0.3; s.rotation += s.spin; s.life -= 0.01; }
    std::vector<AWP_Shell> alive_shells; alive_shells.reserve(awp_shells.size());
    for (auto &s : awp_shells) if (s.life > 0 && s.y < screen_height() + 50) alive_shells.push_back(s); awp_shells = std::move(alive_shells);
}

void AWP::load_assets()
{
    load_bitmap("awp_img", "../image/weapon/AWP.png");
    load_bitmap("bullet_y", "../image/weapon/Bullet_Yellow.png");
    load_bitmap("muzzle_flash", "../image/weapon/muzzle_flash.png");
    load_bitmap("shell_img", "../image/weapon/shell.png");
    image_ = bitmap_named("awp_img");
    awp_muzzle_flash_img = bitmap_named("muzzle_flash");
    awp_shell_img = bitmap_named("shell_img");
}

void AWP::update(const player_data &player)
{
    if (fire_cooldown_ > 0) fire_cooldown_--;
    double cx = player.player_x + player.player_width / 2;
    double cy = player.player_y + player.player_hight / 2;
    double ang = atan2(mouse_y() - cy, mouse_x() - cx);
    if (mouse_clicked(LEFT_BUTTON) && fire_cooldown_ == 0)
    {
        fire_cooldown_ = fire_interval_;
        awp_is_recoiling = true; awp_recoil_timer = awp_recoil_duration;

        Bullet b; b.x = cx; b.y = cy; b.speed = 120;
        b.dx = cos(ang) * b.speed; b.dy = sin(ang) * b.speed; b.active = true;
        b.weapon_id = 3; b.damage = 300; b.image = bitmap_named("bullet_y");
        b.piercing = false; // cancel penetration
        bullets_.push_back(b);

        // Muzzle flash at muzzle point
        awp_muzzle_x = player.player_x + cos(ang) * 45;
        awp_muzzle_y = player.player_y + sin(ang) * 45;
        awp_muzzle_timer = 4;

        // Shell ejection
        AWP_Shell s; s.x = player.player_x; s.y = player.player_y + 15;
        double ej = (player.facing == FACING_RIGHT) ? (3.14 - 0.5 + ((rand() % 20) / 100.0)) : (-0.5 + ((rand() % 20) / 100.0));
        double spd = 5 + rand() % 3; s.dx = cos(ej) * spd; s.dy = sin(ej) * spd;
        s.rotation = rand() % 360; s.spin = (rand() % 20 - 10) * 0.2; s.life = 1.0; s.image = awp_shell_img;
        awp_shells.push_back(s);

        // Fire sound
        load_sound_effect("awp_fire", "../sound/weapon/AWP.mp3");
        play_sound_effect("awp_fire", 0.45);
    }

    if (awp_is_recoiling)
    {
        awp_recoil_timer--; if (awp_recoil_timer <= 0) { awp_is_recoiling = false; awp_recoil_timer = 0; }
    }

    awp_update_bullets(bullets_);
}

void AWP::draw(const player_data &player)
{
    // Draw weapon with recoil offset
    double weapon_x = player.player_x;
    double weapon_y = player.player_y + 32;
    double angle_rad = atan2(mouse_y() - weapon_y, mouse_x() - weapon_x);
    double angle_deg = angle_rad * 180.0 / AWP_PI;
    double recoil_offset = 0.0;
    if (awp_is_recoiling)
    {
        double t = static_cast<double>(awp_recoil_timer) / awp_recoil_duration;
        recoil_offset = awp_recoil_strength * t;
    }
    double draw_x = weapon_x + cos(angle_rad + 3.14159) * recoil_offset;
    double draw_y = weapon_y + sin(angle_rad + 3.14159) * recoil_offset;
    drawing_options opts;
    if (player.facing == FACING_RIGHT)
    {
        if (mouse_x() < player.player_x) opts = option_flip_y(option_rotate_bmp(angle_deg + 180));
        else opts = option_rotate_bmp(angle_deg);
        opts = option_flip_x(opts);
    }
    else
    {
        if (mouse_x() > player.player_x) opts = option_flip_y(option_rotate_bmp(angle_deg + 180));
        else opts = option_rotate_bmp(angle_deg);
    }
    if (image_) draw_bitmap(image_, draw_x, draw_y, opts);

    // Muzzle flash
    if (awp_muzzle_timer > 0 && awp_muzzle_flash_img)
    {
        draw_bitmap(awp_muzzle_flash_img, awp_muzzle_x + 20, awp_muzzle_y + 25, option_rotate_bmp(angle_deg));
        awp_muzzle_timer--;
    }

    // Draw bullets, sparks, shells
    for (const auto &b : bullets_)
    {
        if (!b.active || !b.image) continue;
        double angle_deg_b = atan2(b.dy, b.dx) * 180.0 / AWP_PI;
        draw_bitmap(b.image, b.x, b.y, option_rotate_bmp(angle_deg_b));
    }
    for (const auto &s : awp_sparks)
    {
        color c = rgba_color(255, 230, 60, (int)(s.life * 255));
        double tail_x = s.x - cos(atan2(s.dy, s.dx)) * s.length;
        double tail_y = s.y - sin(atan2(s.dy, s.dx)) * s.length;
        draw_line(c, s.x, s.y, tail_x, tail_y);
    }
    for (const auto &s : awp_shells)
    {
        double scale = 0.5 + 0.5 * s.life;
        draw_bitmap(s.image, s.x, s.y, option_scale_bmp(scale, scale, option_rotate_bmp(s.rotation)));
    }
}
