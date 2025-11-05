#include "weapon_base.hpp"
#include <cmath>
#include <vector>
double M_PI = 3.141592654;
// Global spark system
std::vector<Spark> sparks; // Spark list (global shared)

// Generate sparks: scatter randomly opposite to bullet direction
static void create_sparks(double x, double y, double bullet_angle_rad)
{
    int count = 5 + rand() % 4; // 5~8 sparks
    for (int i = 0; i < count; ++i)
    {
        Spark s;
        s.x = x;
        s.y = y;

        double spread = ((rand() % 61) - 30) * (M_PI / 180.0); // ±30° scatter
        double dir = bullet_angle_rad + M_PI + spread;         // Opposite direction
        double speed = 5 + rand() % 6;

        s.dx = cos(dir) * speed;
        s.dy = sin(dir) * speed;
        s.length = 8 + rand() % 12; // Spark length
        s.life = 1.0;               // Initial life
        sparks.push_back(s);
    }
}

// Shell system
struct Shell
{
    double x, y;     // Position
    double dx, dy;   // Velocity
    double rotation; // Current rotation angle
    double spin;     // Spin speed
    double life;     // Remaining life (1~0)
    bitmap image;    // Shell sprite
};

static std::vector<Shell> shells; // Shell container
static bitmap shell_img;          // Shell sprite

// Weapon internal state
bitmap current_image[2];         // Weapon frames: idle / firing
static bitmap muzzle_flash_img;  // Muzzle flash sprite
static int muzzle_timer = 0;     // Muzzle flash display timer
static int fire_frame_timer = 0; // Firing frame timer
static double muzzle_x = 0, muzzle_y = 0;

// Update bullets
static void update_bullets_vec(std::vector<Bullet> &arr)
{
    for (auto &b : arr)
    {
        bool prev = b.was_active;
        b.was_active = b.active;

        // Generate sparks when bullet becomes inactive
        if (prev && !b.active)
        {
            create_sparks(b.x, b.y, atan2(b.dy, b.dx));
            continue;
        }

        if (!b.active)
            continue;

        // Bullet movement
        b.x += b.dx;
        b.y += b.dy;

        // Deactivate when out of screen
        if (b.x < 0 || b.x > screen_width() || b.y < 0 || b.y > screen_height())
            b.active = false;
    }

    // Update sparks
    for (auto &s : sparks)
    {
        s.x += s.dx;
        s.y += s.dy;
        s.life -= 0.05;
    }

    // Clean up dead sparks
    std::vector<Spark> alive;
    alive.reserve(sparks.size());
    for (auto &s : sparks)
        if (s.life > 0)
            alive.push_back(s);
    sparks = std::move(alive);
}

// Draw bullets and sparks
static void draw_bullets_vec(const std::vector<Bullet> &arr)
{
    // Draw bullets
    for (const auto &b : arr)
    {
        if (!b.active || !b.image)
            continue;
        double angle_deg = atan2(b.dy, b.dx) * 180.0 / M_PI;
        draw_bitmap(b.image, b.x, b.y, option_rotate_bmp(angle_deg));
    }

    // Draw sparks
    for (const auto &s : sparks)
    {
        color c = rgba_color(255, 220 + rand() % 35, 50, (int)(s.life * 255));
        double tail_x = s.x - cos(atan2(s.dy, s.dx)) * s.length;
        double tail_y = s.y - sin(atan2(s.dy, s.dx)) * s.length;
        draw_line(c, s.x, s.y, tail_x, tail_y);
    }

    // Update shell physics
    for (auto &s : shells)
    {
        s.x += s.dx;
        s.y += s.dy;
        s.dy += 0.3; // Gravity
        s.rotation += s.spin;
        s.life -= 0.01; // Decrease life
    }

    // Clean up invalid shells
    std::vector<Shell> alive_shells;
    alive_shells.reserve(shells.size());
    for (auto &s : shells)
        if (s.life > 0 && s.y < screen_height() + 50)
            alive_shells.push_back(s);
    shells = std::move(alive_shells);
}

// Pistol implementation
void Pistol::load_assets()
{
    // Load weapon images
    load_bitmap("weapon_img_0", "../image/weapon/DEagle_0.png");
    load_bitmap("weapon_img_1", "../image/weapon/DEagle_1.png");

    // Load bullet and muzzle flash sprites
    load_bitmap("bullet_0", "../image/weapon/bullet_0.png");
    load_bitmap("muzzle_flash", "../image/weapon/muzzle_flash.png");

    // Load shell sprite
    load_bitmap("shell_img", "../image/weapon/shell.png");
    shell_img = bitmap_named("shell_img");

    // Bind sprite handles
    current_image[0] = bitmap_named("weapon_img_0");
    current_image[1] = bitmap_named("weapon_img_1");
    muzzle_flash_img = bitmap_named("muzzle_flash");
    image_ = current_image[0];
}

void Pistol::update(const player_data &player)
{
    double weapon_x = player.player_x + 15;
    double weapon_y = player.player_y + 15;
    double center_x = player.player_x + player.player_width / 2;
    double center_y = player.player_y + player.player_hight / 2;

    if (fire_cooldown > 0)
        fire_cooldown--;

    double angle_rad = atan2(mouse_y() - center_y, mouse_x() - center_x);

    // Firing logic
    if (mouse_clicked(LEFT_BUTTON) && fire_cooldown == 0 && !is_recoiling)
    {
        muzzle_x = weapon_x + cos(angle_rad) * 45; // Muzzle position X
        muzzle_y = weapon_y + sin(angle_rad) * 45; // Muzzle position Y
        muzzle_timer = 6;                          // Muzzle flash duration
        fire_frame_timer = 6;                      // Firing frame duration

        fire_cooldown = fire_interval; // Reset cooldown
        is_recoiling = true;
        recoil_timer = recoil_duration; // Recoil duration

        // Create bullet
        Bullet b;
        b.x = center_x;
        b.y = center_y;
        b.speed = 50; // Bullet speed
        b.dx = cos(angle_rad) * b.speed;
        b.dy = sin(angle_rad) * b.speed;
        b.active = true;
        b.weapon_id = 0;
        b.damage = 100; // Bullet damage
        b.image = bitmap_named("bullet_0");
        bullets_.push_back(b);

        // Generate shell
        Shell s;
        s.x = weapon_x;
        s.y = weapon_y + 15;

        double ejection_angle;
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
        double speed = 5 + rand() % 3; // Shell speed
        s.dx = cos(ejection_angle) * speed;
        s.dy = sin(ejection_angle) * speed;
        s.rotation = rand() % 360;         // Initial rotation
        s.spin = (rand() % 20 - 10) * 0.2; // Spin speed
        s.life = 1.0;                      // Initial life
        s.image = shell_img;
        shells.push_back(s);

        // Play fire sound
        load_sound_effect("fire", "../sound/weapon/DEagle.mp3");
        play_sound_effect("fire", 0.3); // Volume 0.3

        // Switch to firing frame
        image_ = current_image[1];
    }

    // Recoil recovery
    if (is_recoiling)
    {
        recoil_timer--;
        if (recoil_timer <= 0)
        {
            is_recoiling = false;
            recoil_timer = 0;
        }
    }

    // Weapon frame recovery
    if (fire_frame_timer > 0)
    {
        fire_frame_timer--;
        image_ = current_image[1];
    }
    else
    {
        image_ = current_image[0];
    }

    update_bullets_vec(bullets_);
}

void Pistol::draw(const player_data &player)
{
    double weapon_x = player.player_x + 15;
    double weapon_y = player.player_y + 25;
    double angle_rad = atan2(mouse_y() - weapon_y, mouse_x() - weapon_x);
    double angle_deg = angle_rad * 180.0 / M_PI;

    // Recoil offset
    double recoil_offset = 0.0;
    if (is_recoiling)
    {
        double t = static_cast<double>(recoil_timer) / recoil_duration;
        recoil_offset = recoil_strength * t; // Offset based on recoil strength
    }

    double draw_x = weapon_x + cos(angle_rad + M_PI) * recoil_offset;
    double draw_y = weapon_y + sin(angle_rad + M_PI) * recoil_offset;

    // Draw weapon
    drawing_options opts = option_rotate_bmp(angle_deg);
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
    }
    else
    {
        if (mouse_x() > player.player_x)
            opts = option_flip_y(option_rotate_bmp(angle_deg + 180));
        else
            opts = option_rotate_bmp(angle_deg);
    }
    draw_bitmap(image_, draw_x, draw_y, opts);

    // Draw muzzle flash
    if (muzzle_timer > 0 && muzzle_flash_img != nullptr)
    {
        if (player.facing == FACING_RIGHT)
        {
            if (mouse_x() < player.player_x)
                draw_bitmap(muzzle_flash_img, muzzle_x + 25, muzzle_y + 8, option_rotate_bmp(angle_deg));
            else
                draw_bitmap(muzzle_flash_img, muzzle_x - 6, muzzle_y + 5, option_rotate_bmp(angle_deg));
        }
        else
        {
            if (mouse_x() > player.player_x)
                draw_bitmap(muzzle_flash_img, muzzle_x + 10, muzzle_y + 5, option_rotate_bmp(angle_deg));
            else
                draw_bitmap(muzzle_flash_img, muzzle_x + 15, muzzle_y + 5, option_rotate_bmp(angle_deg));
        }
        muzzle_timer--;
    }

    // Draw bullets + sparks + shells
    draw_bullets_vec(bullets_);

    // Shells: fade out with scale
    for (const auto &s : shells)
    {
        double scale = 0.5 + 0.5 * s.life; // Scale based on life
        draw_bitmap(s.image, s.x, s.y, option_scale_bmp(scale, scale, option_rotate_bmp(s.rotation)));
    }
}