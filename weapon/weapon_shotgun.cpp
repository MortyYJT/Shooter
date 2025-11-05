// Shotgun: fixed spread pellets, muzzle flash, shells, sparks on destroy, AK-like recoil.
#include "weapon_base.hpp"
#include <cmath>
#include <vector>

static double SG_PI = 3.141592654;

// VFX state
static std::vector<Spark> sg_sparks;
struct SG_Shell { double x,y,dx,dy,rotation,spin,life; bitmap image; };
static std::vector<SG_Shell> sg_shells;
static bitmap sg_shell_img;
static bitmap sg_muzzle_img;
static int sg_muzzle_timer = 0; static double sg_mx=0, sg_my=0;

// Recoil (AK-like)
static bool sg_recoiling = false; static int sg_recoil_timer = 0;
static const int sg_recoil_duration = 20; static const double sg_recoil_strength = 8.0;

static void sg_create_sparks(double x,double y,double ang)
{
    int cnt = 5 + rand()%3; for (int i=0;i<cnt;++i){ Spark s; s.x=x; s.y=y; double sp=((rand()%41)-20)*(SG_PI/180.0); double dir=ang+SG_PI+sp; double v=5+rand()%6; s.dx=cos(dir)*v; s.dy=sin(dir)*v; s.length=8+rand()%12; s.life=1.0; sg_sparks.push_back(s);} }

static void sg_update_bullets(std::vector<Bullet>& arr){
    for (auto &b:arr){ bool prev=b.was_active; b.was_active=b.active; if(prev&&!b.active){ sg_create_sparks(b.x,b.y,atan2(b.dy,b.dx)); continue;} if(!b.active) continue; b.x+=b.dx; b.y+=b.dy; if(b.x<0||b.x>screen_width()||b.y<0||b.y>screen_height()) b.active=false; }
    for(auto &s:sg_sparks){ s.x+=s.dx; s.y+=s.dy; s.life-=0.05; }
    std::vector<Spark> alive; for(auto&s:sg_sparks) if(s.life>0) alive.push_back(s); sg_sparks=std::move(alive);
    for(auto&s:sg_shells){ s.x+=s.dx; s.y+=s.dy; s.dy+=0.3; s.rotation+=s.spin; s.life-=0.01; }
    std::vector<SG_Shell> alive2; for(auto&s:sg_shells) if(s.life>0 && s.y<screen_height()+50) alive2.push_back(s); sg_shells=std::move(alive2);
}

void Shotgun::load_assets()
{
    load_bitmap("shotgun_img", "../image/weapon/Shotgun.png");
    load_bitmap("bullet_fire", "../image/weapon/Bullet_fire.png");
    load_bitmap("muzzle_flash", "../image/weapon/muzzle_flash.png");
    load_bitmap("shell_img", "../image/weapon/shell.png");
    image_ = bitmap_named("shotgun_img");
    sg_muzzle_img = bitmap_named("muzzle_flash");
    sg_shell_img = bitmap_named("shell_img");
}

void Shotgun::update(const player_data &player)
{
    if (fire_cooldown_ > 0) fire_cooldown_--;

    double cx = player.player_x + player.player_width / 2;
    double cy = player.player_y + player.player_hight / 2;
    double ang = atan2(mouse_y() - cy, mouse_x() - cx);

    if (mouse_clicked(LEFT_BUTTON) && fire_cooldown_ == 0)
    {
        fire_cooldown_ = fire_interval_;
        sg_recoiling = true; sg_recoil_timer = sg_recoil_duration;

        // Reduced pellet count, still denser near center
        const double offs_deg[] = {-10,-5,-2,0,2,5,10};
        int pellets = sizeof(offs_deg)/sizeof(offs_deg[0]);
        for (int i=0;i<pellets;++i)
        {
            double a = ang + offs_deg[i] * (SG_PI/180.0);
            Bullet b; b.x = cx; b.y = cy; b.speed = 50; b.dx = cos(a)*b.speed; b.dy = sin(a)*b.speed;
            b.active = true; b.weapon_id = 2; b.damage = 24; b.image = bitmap_named("bullet_fire");
            bullets_.push_back(b);
        }

        // VFX: muzzle + shell
        sg_mx = player.player_x + cos(ang) * 45; sg_my = player.player_y + sin(ang) * 45; sg_muzzle_timer = 4;
        SG_Shell s; s.x = player.player_x; s.y = player.player_y + 15; double ej = (player.facing==FACING_RIGHT)?(3.14-0.5+((rand()%20)/100.0)):(-0.5+((rand()%20)/100.0)); double spd=5+rand()%3; s.dx=cos(ej)*spd; s.dy=sin(ej)*spd; s.rotation=rand()%360; s.spin=(rand()%20-10)*0.2; s.life=1.0; s.image=sg_shell_img; sg_shells.push_back(s);

        // Sound
        load_sound_effect("shotgun_fire","../sound/weapon/shotgun.mp3");
        play_sound_effect("shotgun_fire",0.5);
    }

    if (sg_recoiling) { sg_recoil_timer--; if (sg_recoil_timer<=0){ sg_recoiling=false; sg_recoil_timer=0; } }

    sg_update_bullets(bullets_);
}

void Shotgun::draw(const player_data &player)
{
    double weapon_x = player.player_x;
    double weapon_y = player.player_y + 32;
    double angle_rad = atan2(mouse_y() - weapon_y, mouse_x() - weapon_x);
    double angle_deg = angle_rad * 180.0 / SG_PI;
    double recoil_offset = 0.0;
    if (sg_recoiling) { double t = static_cast<double>(sg_recoil_timer)/sg_recoil_duration; recoil_offset = sg_recoil_strength * t; }
    double draw_x = weapon_x + cos(angle_rad + 3.14159) * recoil_offset;
    double draw_y = weapon_y + sin(angle_rad + 3.14159) * recoil_offset;
    drawing_options opts;
    if (player.facing == FACING_RIGHT)
    { if (mouse_x() < player.player_x) opts = option_flip_y(option_rotate_bmp(angle_deg + 180)); else opts = option_rotate_bmp(angle_deg); opts = option_flip_x(opts); }
    else { if (mouse_x() > player.player_x) opts = option_flip_y(option_rotate_bmp(angle_deg + 180)); else opts = option_rotate_bmp(angle_deg); }
    if (image_) draw_bitmap(image_, draw_x, draw_y, opts);

    if (sg_muzzle_timer>0 && sg_muzzle_img) { draw_bitmap(sg_muzzle_img, sg_mx+20, sg_my+25, option_rotate_bmp(angle_deg)); sg_muzzle_timer--; }

    for (const auto &b : bullets_)
    { if (!b.active || !b.image) continue; double angle_deg_b = atan2(b.dy,b.dx) * 180.0 / SG_PI; draw_bitmap(b.image, b.x, b.y, option_rotate_bmp(angle_deg_b)); }
    for (const auto &s : sg_sparks)
    { color c = rgba_color(255, 200, 40, (int)(s.life*255)); double tx = s.x - cos(atan2(s.dy,s.dx))*s.length; double ty = s.y - sin(atan2(s.dy,s.dx))*s.length; draw_line(c, s.x, s.y, tx, ty); }
    for (const auto &s : sg_shells)
    { double sc = 0.5 + 0.5*s.life; draw_bitmap(s.image, s.x, s.y, option_scale_bmp(sc, sc, option_rotate_bmp(s.rotation))); }
}
