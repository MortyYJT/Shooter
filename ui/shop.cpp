// Shop UI implementation
#include "shop.hpp"

static void preload_images(const ShopState &s)
{
    for (auto &it : s.items) load_bitmap(it.image_key, it.image_path);
}

void init_shop(ShopState &s)
{
    s.items = {
        {"AK-47","weapon_ak_img","../image/weapon/weapon_AK-47.png",0,"","",1,true},
        {"Pistol","weapon_img_0","../image/weapon/DEagle_0.png",0,"","",0,true},
        {"Shotgun","shotgun_img","../image/weapon/Shotgun.png",80,"","",2,false},
        {"AWP","awp_img","../image/weapon/AWP.png",120,"","",3,false}
    };
    s.dragging = false;
    s.drag_index = -1;
    s.mouse_prev_down = false;
    s.suppress_drag_until_release = false;
    s.money_warn_timer = 0;
    preload_images(s);
}

bool update_shop(ShopState &s,
                 std::vector<std::unique_ptr<WeaponBase>> &weapons,
                 int &current_weapon,
                 int &money)
{
    bool now_down = mouse_down(LEFT_BUTTON);
    bool released_now = (!now_down && s.mouse_prev_down);
    bool clicked_now = (now_down && !s.mouse_prev_down);
    if (s.money_warn_timer > 0) s.money_warn_timer--;

    // Card layout (grid)
    const double start_x = 240;
    const double start_y = 260;
    const double gap_x = 140;
    const double gap_y = 24;
    const double card_w = 170;
    const double card_h = 140;
    const int cols = 3;

    // Determine hovered card
    int hovered_idx = -1;
    double mx = mouse_x(), my = mouse_y();
    for (int i = 0; i < (int)s.items.size(); ++i)
    {
        int col = i % cols;
        int row = i / cols;
        double cx = start_x + col * (card_w + gap_x);
        double cy = start_y + row * (card_h + gap_y);
        if (mx >= cx && mx <= cx + card_w && my >= cy && my <= cy + card_h)
        {
            hovered_idx = i;
            break;
        }
    }

    // Unlock on click
    if (clicked_now && hovered_idx >= 0 && !s.items[hovered_idx].unlocked)
    {
        if (money >= s.items[hovered_idx].price)
        {
            money -= s.items[hovered_idx].price;
            s.items[hovered_idx].unlocked = true;
            s.suppress_drag_until_release = true;
        }
        else
        {
            s.money_warn_timer = 30; // trigger warning animation
        }
    }

    // Start drag for unlocked items
    if (now_down && !s.dragging && hovered_idx >= 0 && s.items[hovered_idx].unlocked && !s.suppress_drag_until_release)
    {
        s.dragging = true;
        s.drag_index = hovered_idx;
        s.drag_x = mx;
        s.drag_y = my;
    }

    if (s.dragging)
    {
        s.drag_x = mx;
        s.drag_y = my;
        if (released_now)
        {
            s.suppress_drag_until_release = false;

            // Slot layout (same as draw)
            double slot_y = 620;
            double slot_w = 144;
            double slot_h = 120;
            double slot_x1 = 380;
            double slot_x2 = 760;
            int target_slot = -1;
            if (s.drag_x >= slot_x1 && s.drag_x <= slot_x1 + slot_w && s.drag_y >= slot_y && s.drag_y <= slot_y + slot_h)
                target_slot = 0;
            if (s.drag_x >= slot_x2 && s.drag_x <= slot_x2 + slot_w && s.drag_y >= slot_y && s.drag_y <= slot_y + slot_h)
                target_slot = 1;

            if (target_slot != -1 && s.drag_index >= 0)
            {
                auto &it = s.items[s.drag_index];
                if (it.unlocked)
                {
                    std::unique_ptr<WeaponBase> nw;
                    if (it.type == 0)
                        nw = std::make_unique<Pistol>();
                    else if (it.type == 1)
                        nw = std::make_unique<AK>();
                    else if (it.type == 2)
                        nw = std::make_unique<Shotgun>();
                    else if (it.type == 3)
                        nw = std::make_unique<AWP>();
                    

                    if (nw)
                    {
                        nw->load_assets();
                        if (weapons.size() < 2)
                            weapons.resize(2);
                        int other = target_slot == 0 ? 1 : 0;
                        bool same_type = false;
                        if (weapons.size() > other && weapons[other])
                        {
                            if ((it.type == 0 && dynamic_cast<Pistol *>(weapons[other].get())) ||
                                (it.type == 1 && dynamic_cast<AK *>(weapons[other].get())) ||
                                (it.type == 2 && dynamic_cast<Shotgun *>(weapons[other].get())) ||
                                (it.type == 3 && dynamic_cast<AWP *>(weapons[other].get())))
                                same_type = true;
                        }
                        if (!same_type)
                        {
                            weapons[target_slot] = std::move(nw);
                            current_weapon = target_slot;
                        }
                    }
                }
            }

            s.dragging = false;
            s.drag_index = -1;
        }
    }

    if (!now_down)
        s.suppress_drag_until_release = false;

    if (key_typed(ESCAPE_KEY) || key_typed(SPACE_KEY) || key_typed(RETURN_KEY))
    {
        s.mouse_prev_down = now_down;
        return true;
    }

    s.mouse_prev_down = now_down;
    return false;
}

void draw_shop(const ShopState &s,
               const std::vector<std::unique_ptr<WeaponBase>> &weapons,
               int money,
               int screen_w)
{
    fill_rectangle(rgba_color(255,255,255,220), 200, 160, 1200, 640);
    draw_text("Tip: Click to unlock -> drag to slots; 1/2 to switch; B open/close shop; Enter start next wave; ESC close shop", COLOR_BLACK, "arial", 24, 230, 190);
    {
        int money_y = 160 + 640 - 40;
        int money_x = 200 + 1200 - 240;
        if (s.money_warn_timer > 0)
        {
            double phase = (30 - s.money_warn_timer) * 0.6;
            money_y += (int)(sin(phase) * 8);
        }
        color money_col = (s.money_warn_timer > 0) ? COLOR_RED : COLOR_BLACK;
        draw_text("Money: $" + std::to_string(money), money_col, "arial", 26, money_x, money_y);
    }

    double start_x = 240, start_y = 260, gap_x = 140, gap_y = 24, card_w = 170, card_h = 140;
    int cols = 3;
    for (int i = 0; i < (int)s.items.size(); ++i)
    {
        int col = i % cols, row = i / cols;
        double cx = start_x + col * (card_w + gap_x);
        double cy = start_y + row * (card_h + gap_y);
        draw_rectangle(COLOR_BLACK, cx, cy, card_w, card_h);
        bitmap img = bitmap_named(s.items[i].image_key);
        if (img)
        {
            double iw = bitmap_width(img) * 2.0;
            double ih = bitmap_height(img) * 2.0;
            double ix = cx + (card_w - iw) / 2.0;
            double iy = cy + (card_h - ih) / 2.0 - 6;
            draw_bitmap(img, ix, iy, option_scale_bmp(2.0, 2.0, option_flip_x()));
        }
        color tc = s.items[i].unlocked ? COLOR_GREEN : COLOR_RED;
        draw_text(s.items[i].name + (s.items[i].unlocked?" (Unlocked)":""), tc, "arial", 18, cx + 6, cy + card_h - 40);
        draw_text("$" + std::to_string(s.items[i].price), COLOR_BLACK, "arial", 18, cx + 6, cy + card_h - 24);
    }

    double slot_y = 620; double slot_w = 144; double slot_h = 120; double slot_x1 = 380; double slot_x2 = 760;
    draw_rectangle(COLOR_BLACK, slot_x1, slot_y, slot_w, slot_h);
    draw_rectangle(COLOR_BLACK, slot_x2, slot_y, slot_w, slot_h);

    auto draw_slot_img = [](bitmap img, double x, double y, double w, double h)
    {
        if (!img) return;
        double scale = 1.5;
        double iw = bitmap_width(img) * scale;
        double ih = bitmap_height(img) * scale;
        double ix = x + (w - iw) / 2.0;
        double iy = y + (h - ih) / 2.0;
        draw_bitmap(img, ix, iy, option_scale_bmp(scale, scale, option_flip_x()));
    };

    if (weapons.size() > 0 && weapons[0])
    {
        bitmap img = nullptr;
        if (dynamic_cast<Pistol*>(weapons[0].get())) img = bitmap_named("weapon_img_0");
        else if (dynamic_cast<AK*>(weapons[0].get())) img = bitmap_named("weapon_ak_img");
        else if (dynamic_cast<Shotgun*>(weapons[0].get())) img = bitmap_named("shotgun_img");
        else if (dynamic_cast<AWP*>(weapons[0].get())) img = bitmap_named("awp_img");
        draw_slot_img(img, slot_x1, slot_y, slot_w, slot_h);
        draw_text("Slot 1", COLOR_BLACK, "arial", 18, slot_x1 + 8, slot_y + 8);
    }
    if (weapons.size() > 1 && weapons[1])
    {
        bitmap img = nullptr;
        if (dynamic_cast<Pistol*>(weapons[1].get())) img = bitmap_named("weapon_img_0");
        else if (dynamic_cast<AK*>(weapons[1].get())) img = bitmap_named("weapon_ak_img");
        else if (dynamic_cast<Shotgun*>(weapons[1].get())) img = bitmap_named("shotgun_img");
        else if (dynamic_cast<AWP*>(weapons[1].get())) img = bitmap_named("awp_img");
        draw_slot_img(img, slot_x2, slot_y, slot_w, slot_h);
        draw_text("Slot 2", COLOR_BLACK, "arial", 18, slot_x2 + 8, slot_y + 8);
    }

    if (s.dragging && s.drag_index >= 0)
    {
        bitmap img = bitmap_named(s.items[s.drag_index].image_key);
        if (img)
            draw_bitmap(img, s.drag_x - bitmap_width(img), s.drag_y - bitmap_height(img), option_scale_bmp(2.0, 2.0, option_flip_x()));
    }
}
