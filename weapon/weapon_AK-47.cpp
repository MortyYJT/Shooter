#include "weapon_base.hpp"
#include <cmath>
#include <vector>

double AK_M_PI = 3.141592654; // Approximation of pi for AK calculations

// Similar to Pistol effects, with "ak_" prefix to avoid symbol conflicts
static std::vector<Spark> ak_sparks; // Sparks for AK
struct AK_Shell
{
    double x, y; // Position coordinates
    double dx, dy; // Velocity components
    double rotation; // Rotation angle
    double spin; // Rotation speed
    double life; // Lifespan (0-1)
    bitmap image; // Shell texture
};
static std::vector<AK_Shell> ak_shells; // Shells for AK
static bitmap ak_shell_img; // Image for AK shells

static bitmap ak_muzzle_flash_img; // Muzzle flash image for AK
static int ak_muzzle_timer = 0; // Timer for muzzle flash display
static int ak_fire_frame_timer = 0; // Timer for fire animation frame
static double ak_muzzle_x = 0, ak_muzzle_y = 0; // Muzzle position coordinates

// Create sparks (similar to Pistol)
static void ak_create_sparks(double x, double y, double bullet_angle_rad) // x,y: spark origin; bullet_angle_rad: bullet direction in radians
{
    int count = 5 + rand() % 4;
    for (int i = 0; i < count; ++i)
    {
        Spark s;
        s.x = x;
        s.y = y;
        double spread = ((rand() % 61) - 30) * (AK_M_PI / 180.0); // Random angle spread (deg to rad)
        double dir = bullet_angle_rad + AK_M_PI + spread; // Spark direction
        double speed = 5 + rand() % 6; // Spark speed
        s.dx = cos(dir) * speed;
        s.dy = sin(dir) * speed;
        s.length = 8 + rand() % 12; // Spark length
        s.life = 1.0; // Initial lifespan
        ak_sparks.push_back(s);
    }
}

// Update AK bullets, sparks (same behavior as Pistol's update_bullets_vec)
static void ak_update_bullets_vec(std::vector<Bullet> &arr) // arr: bullet vector to update
{
    for (auto &b : arr)
    {
        bool prev = b.was_active;
        b.was_active = b.active;

        if (prev && !b.active)
        {
            ak_create_sparks(b.x, b.y, atan2(b.dy, b.dx));
            continue;
        }

        if (!b.active)
            continue;

        b.x += b.dx;
        b.y += b.dy;

        // Deactivate if out of screen bounds
        if (b.x < 0 || b.x > screen_width() || b.y < 0 || b.y > screen_height())
            b.active = false;
    }

    // Update sparks
    for (auto &s : ak_sparks)
    {
        s.x += s.dx;
        s.y += s.dy;
        s.life -= 0.05; // Decrease lifespan
    }

    // Clean up expired sparks
    std::vector<Spark> alive;
    alive.reserve(ak_sparks.size());
    for (auto &s : ak_sparks)
        if (s.life > 0)
            alive.push_back(s);
    ak_sparks = std::move(alive);

    // Update shell physics
    for (auto &s : ak_shells)
    {
        s.x += s.dx;
        s.y += s.dy;
        s.dy += 0.3; // Apply gravity
        s.rotation += s.spin; // Update rotation
        s.life -= 0.01; // Decrease lifespan
    }

    // Clean up expired shells
    std::vector<AK_Shell> alive_shells;
    alive_shells.reserve(ak_shells.size());
    for (auto &s : ak_shells)
        if (s.life > 0 && s.y < screen_height() + 50) // Keep if alive and within bounds
            alive_shells.push_back(s);
    ak_shells = std::move(alive_shells);
}

// Draw AK bullets, sparks, shells (similar to Pistol's draw_bullets_vec)
static void ak_draw_bullets_vec(const std::vector<Bullet> &arr) // arr: bullet vector to draw
{
    for (const auto &b : arr)
    {
        if (!b.active || !b.image)
            continue;
        double angle_deg = atan2(b.dy, b.dx) * 180.0 / AK_M_PI; // Convert rad to deg
        draw_bitmap(b.image, b.x, b.y, option_rotate_bmp(angle_deg));
    }

    for (const auto &s : ak_sparks)
    {
        color c = rgba_color(255, 220 + rand() % 35, 50, (int)(s.life * 255)); // Spark color with alpha
        double tail_x = s.x - cos(atan2(s.dy, s.dx)) * s.length; // Spark tail position
        double tail_y = s.y - sin(atan2(s.dy, s.dx)) * s.length;
        draw_line(c, s.x, s.y, tail_x, tail_y);
    }

    for (const auto &s : ak_shells)
    {
        double scale = 0.5 + 0.5 * s.life; // Scale based on lifespan
        draw_bitmap(s.image, s.x, s.y, option_scale_bmp(scale, scale, option_rotate_bmp(s.rotation)));
    }
}

// ==============================
// AK-47 implementation (based on Pistol structure)
// ==============================
void AK::load_assets()
{
    // Weapon texture
    load_bitmap("weapon_ak_img", "../image/weapon/weapon_AK-47.png");
    image_ = bitmap_named("weapon_ak_img");

    // Shared bullet/flash/shell textures (load if not present)
    load_bitmap("bullet_0", "../image/weapon/bullet_0.png");
    load_bitmap("muzzle_flash", "../image/weapon/muzzle_flash.png");
    load_bitmap("shell_img", "../image/weapon/shell.png");

    ak_muzzle_flash_img = bitmap_named("muzzle_flash");
    ak_shell_img = bitmap_named("shell_img");
}

void AK::update(const player_data &player) // player: player state data
{
    double weapon_x = player.player_x; // Weapon X position
    double weapon_y = player.player_y; // Weapon Y position
    double center_x = player.player_x + player.player_width / 2; // Player center X
    double center_y = player.player_y + player.player_hight / 2; // Player center Y

    if (fire_cooldown_ > 0)
        fire_cooldown_--; // Decrease fire cooldown

    // Angle from player center to mouse (align with crosshair)
    double angle_rad = atan2(mouse_y() - center_y, mouse_x() - center_x);

    // Full-auto: fire continuously while left mouse held (diff from Pistol)
    if (mouse_down(LEFT_BUTTON) && fire_cooldown_ == 0)
    {
        fire_cooldown_ = fire_interval_; // Reset cooldown
        is_recoiling_ = true;
        recoil_timer_ = recoil_duration_; // Reset recoil timer

        // Spawn bullet (set speed first)
        Bullet b;
        b.x = center_x;
        b.y = center_y;
        b.speed = 100; // AK bullet speed
        b.dx = cos(angle_rad) * b.speed; // X velocity
        b.dy = sin(angle_rad) * b.speed; // Y velocity
        b.active = true; // Activate bullet
        b.weapon_id = 1; // Weapon identifier
        b.damage = 70; // Bullet damage
        b.image = bitmap_named("bullet_0");
        bullets_.push_back(b);

        // Set muzzle position and flash timer (copy Pistol logic)
        ak_muzzle_x = weapon_x + cos(angle_rad) * 45;
        ak_muzzle_y = weapon_y + sin(angle_rad) * 45;
        ak_muzzle_timer = 4; // Muzzle flash duration
        ak_fire_frame_timer = 4; // Fire frame duration

        // Spawn shell (same logic as Pistol)
        AK_Shell s;
        s.x = weapon_x;
        s.y = weapon_y + 15;

        double ejection_angle; // Shell ejection angle
        if (player.facing == FACING_RIGHT)
        {
            if (mouse_x() < player.player_x)
                ejection_angle = -0.5 + ((rand() % 20) / 100.0);
            else
                ejection_angle = 3.14 - 0.5 + ((rand() % 20) / 100.0);
        }
        else
        {
            if (mouse_x() > player.player_x)
                ejection_angle = 3.14 - 0.5 + ((rand() % 20) / 100.0);
            else
                ejection_angle = -0.5 + ((rand() % 20) / 100.0);
        }
        double spd = 5 + rand() % 3; // Shell ejection speed
        s.dx = cos(ejection_angle) * spd;
        s.dy = sin(ejection_angle) * spd;
        s.rotation = rand() % 360; // Initial rotation
        s.spin = (rand() % 20 - 10) * 0.2; // Rotation speed
        s.life = 1.0; // Initial lifespan
        s.image = ak_shell_img;
        ak_shells.push_back(s);

        // Play AK fire sound (replace with actual file)
        load_sound_effect("ak_fire", "../sound/weapon/AK47.mp3");
        play_sound_effect("ak_fire", 0.70); // Volume doubled to ~200%
    }

    // Recoil recovery
    if (is_recoiling_)
    {
        recoil_timer_--;
        if (recoil_timer_ <= 0)
        {
            is_recoiling_ = false;
            recoil_timer_ = 0;
        }
    }

    // Fire frame recovery (if using fire animation frames)
    if (ak_fire_frame_timer > 0)
    {
        ak_fire_frame_timer--;
    }

    // Update bullets, sparks, shells
    ak_update_bullets_vec(bullets_);
}

void AK::draw(const player_data &player) // player: player state data
{
    double weapon_x = player.player_x; // Weapon X position
    double weapon_y = player.player_y + 32; // Weapon Y position

    double angle_rad = atan2(mouse_y() - weapon_y, mouse_x() - weapon_x); // Angle in radians
    double angle_deg = angle_rad * 180.0 / 3.14159265358979323846; // Convert to degrees

    // Recoil offset
    double recoil_offset = 0.0;
    if (is_recoiling_)
    {
        double t = static_cast<double>(recoil_timer_) / recoil_duration_; // Recoil progress (0-1)
        recoil_offset = recoil_strength_ * t; // Calculate offset
    }

    double draw_x = weapon_x + cos(angle_rad + 3.14159) * recoil_offset; // Adjusted draw X
    double draw_y = weapon_y + sin(angle_rad + 3.14159) * recoil_offset; // Adjusted draw Y

    drawing_options opts = option_rotate_bmp(angle_deg); // Drawing options
    if (player.facing == FACING_RIGHT)
    {
        if (mouse_x() < player.player_x)
        {
            opts = option_flip_y(option_rotate_bmp(angle_deg + 180));
            opts = option_flip_x(opts);
        }
        else
        {
            opts = option_rotate_bmp(angle_deg);
            opts = option_flip_x(opts);
        }
        draw_bitmap(image_, draw_x, draw_y, opts);
    }
    else
    {
        if (mouse_x() > player.player_x)
            opts = option_flip_y(option_rotate_bmp(angle_deg + 180));
        else
            opts = option_rotate_bmp(angle_deg);

        draw_bitmap(image_, draw_x, draw_y, opts);
    }

    // Draw muzzle flash
    if (ak_muzzle_timer > 0 && ak_muzzle_flash_img != nullptr)
    {
        if (player.facing == FACING_RIGHT)
        {
            if (mouse_x() < player.player_x)
                draw_bitmap(ak_muzzle_flash_img, ak_muzzle_x + 25, ak_muzzle_y + 28, option_rotate_bmp(angle_deg));
            else
                draw_bitmap(ak_muzzle_flash_img, ak_muzzle_x +20, ak_muzzle_y + 25, option_rotate_bmp(angle_deg));
        }
        else
        {
            if (mouse_x() > player.player_x)
                draw_bitmap(ak_muzzle_flash_img, ak_muzzle_x + 15, ak_muzzle_y + 25, option_rotate_bmp(angle_deg));
            else
                draw_bitmap(ak_muzzle_flash_img, ak_muzzle_x + 15, ak_muzzle_y + 25, option_rotate_bmp(angle_deg));
        }
        ak_muzzle_timer--;
    }

    // Draw bullets, sparks, shells
    ak_draw_bullets_vec(bullets_);
}
