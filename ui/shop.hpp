// Simple shop UI module: two-weapon slots, click-to-unlock then drag to equip.
#pragma once

#include <vector>
#include <memory>
#include <string>
#include "splashkit.h"
#include "../weapon/weapon_base.hpp"

struct ShopItem
{
    std::string name;
    std::string image_key;
    std::string image_path;
    int price;
    std::string dmg;
    std::string rof;
    int type;      // 0=Pistol,1=AK,2=Shotgun,3=AWP
    bool unlocked; // true after purchased
};

struct ShopState
{
    bool open = false;
    std::vector<ShopItem> items;

    // Drag state
    bool dragging = false;
    int drag_index = -1;
    double drag_x = 0, drag_y = 0;
    bool mouse_prev_down = false;
    bool suppress_drag_until_release = false; // after unlocking, require new click
    int money_warn_timer = 0; // bounce/red animation when insufficient funds
};

void init_shop(ShopState &s);
inline bool shop_is_open(const ShopState &s) { return s.open; }
inline void open_shop(ShopState &s) { s.open = true; }
inline void close_shop(ShopState &s) { s.open = false; s.dragging = false; s.drag_index = -1; }

// Returns true when user confirms to start next wave (ESC/Enter/Space)
bool update_shop(ShopState &s,
                 std::vector<std::unique_ptr<WeaponBase>> &weapons,
                 int &current_weapon,
                 int &money);

void draw_shop(const ShopState &s,
               const std::vector<std::unique_ptr<WeaponBase>> &weapons,
               int money,
               int screen_w);

