#include "splashkit.h"
#include "player/player.hpp"
#include "weapon/weapon_base.hpp"
#include "enemy/enemy_base.hpp"
#include "enemy/slime/slime.hpp"
#include "enemy/enemy_spawn.hpp"
#include "enemy/boss/boss.hpp"
#include "coin.hpp"
#include "ui/shop.hpp"
#include "ui/menu.hpp"
#include "ui/pause.hpp"
#include "save/save.hpp"
#include <memory> // for std::unique_ptr
#include <vector>
#include <string>
int money = 0;                 // Player's total money
player_data player;               // Global player instance
int camera_shake_timer = 0;       // Screen shake timer
int kill_marker_timer = 0;        // Kill hitmarker timer
int block_flash_timer = 0;        // Block yellow flash timer
int wave = 1;                     // Current wave number
bool wave_in_progress = true;     // Whether current wave is active
int wave_clear_timer = 0;         // Timer for wave-clear delay
const int wave_clear_delay = 360; // Frames between waves
std::vector<Coin> coins;          // Active coin objects
static std::unique_ptr<WeaponBase> create_weapon_by_type(int type)
{
    if (type == 0) return std::make_unique<Pistol>();
    if (type == 1) return std::make_unique<AK>();
    if (type == 2) return std::make_unique<Shotgun>();
    if (type == 3) return std::make_unique<AWP>();
    return nullptr;
}

int main()
{
    // --- Game initialization ---
    load_font("arial", "C:/Windows/Fonts/arial.ttf"); // Load font (Windows path)
    int screen_w = 1600, screen_h = 1200;
    open_window("Shooter - Enemy Test", screen_w, screen_h);
    hide_mouse();
    load_music("bgm", "sound/bgm/bgm_1.mp3");
    // --- Background setup ---
    load_bitmap("background_0", "image/background/background_0.png");
    bitmap background = bitmap_named("background_0");
    // --- Player initialization ---
    player.player_x = screen_w / 2.0;
    player.player_y = screen_h / 2.0;
    player.player_speed = 2;
    load_player(player);
    // Block assets now loaded inside load_player
    load_bitmap("coin_ui", "../image/ui/coin_4.png");
    // --- Weapon system setup ---
    int current_weapon = 0; // active weapon index
    std::vector<std::unique_ptr<WeaponBase>> weapons;
    auto ak = std::make_unique<AK>();
    ak->load_assets();
    weapons.push_back(std::move(ak));
    auto pistol = std::make_unique<Pistol>();
    pistol->load_assets();
    weapons.push_back(std::move(pistol));
    // Shop state and Main menu state
    ShopState shop;
    init_shop(shop);
    MenuState menu;
    init_menu(menu, save_exists());
    // --- Enemy container ---
    std::vector<std::unique_ptr<EnemyBase>> enemies;
    // --- Post-wave coin attract timer ---
    // --- Main game loop ---
    while (!quit_requested())
    {
        hide_mouse();
        process_events();

        bool shop_open = shop_is_open(shop);

        static bool wave_cleared_prompt = false;

        // Main menu gate
        if (menu.in_menu)
        {
            MenuAction act = update_menu(menu);
            draw_bitmap(background, 0, 0);
            draw_menu(menu, screen_w, screen_h);
            refresh_screen(120);

            if (act == MenuAction::NewGame)
            {
                stop_music();
                // reset baseline
                delete_save(); // remove old save so Continue is hidden next time
                money = 0;
                wave = 1;
                wave_in_progress = true;
                wave_clear_timer = 0;
                enemies.clear();
                coins.clear();
                close_shop(shop);
                init_shop(shop);
                // default weapons
                weapons.clear();
                auto ak2 = std::make_unique<AK>(); ak2->load_assets();
                auto pistol2 = std::make_unique<Pistol>(); pistol2->load_assets();
                weapons.push_back(std::move(ak2));
                weapons.push_back(std::move(pistol2));
                // reset player state
                player.alive = true;
                player.max_hearts = 6;
                player.hearts = player.max_hearts;
                player.damage_cooldown = 0;
                player.just_got_hit = false;
                player.knockback_dx = player.knockback_dy = 0;
                player.knockback_timer = 0;
                player.blocking = false;
                player.block_timer = 0;
                player.dash_disabled = false;
                player.player_speed = 2.0;
                play_music("bgm", -1);
                set_music_volume(1.0);
                // start playing
                menu.in_menu = false;
            }
            else if (act == MenuAction::Continue && save_exists())
            {
                SaveData sd; if (load_game(sd))
                {
                    stop_music();
                    money = sd.money;
                    enemies.clear();
                    coins.clear();
                    // restore shop unlocks
                    init_shop(shop);
                    for (int i = 0; i < 6 && i < (int)shop.items.size(); ++i)
                        shop.items[i].unlocked = sd.unlocked[i];
                    // restore weapons
                    weapons.clear();
                    weapons.resize(2);
                    for (int s = 0; s < 2; ++s) {
                        if (sd.slot_types[s] >= 0) {
                            auto w = create_weapon_by_type(sd.slot_types[s]);
                            if (w) { w->load_assets(); weapons[s] = std::move(w); }
                        }
                    }
                    current_weapon = (sd.current_slot == 1 ? 1 : 0);
                    // ensure at least one weapon
                    if (!weapons[0] && !weapons[1]) {
                        auto ak2 = std::make_unique<AK>(); ak2->load_assets();
                        weapons[0] = std::move(ak2);
                        current_weapon = 0;
                    }
                    // if chosen slot is empty, pick the other non-empty slot
                    if (!weapons[current_weapon]) {
                        if (current_weapon == 1 && weapons[0]) current_weapon = 0;
                        else if (current_weapon == 0 && weapons.size() > 1 && weapons[1]) current_weapon = 1;
                    }
                    bool boss_reload = (sd.wave >= 5);
                    if (boss_reload)
                    {
                        wave = 4;
                        wave_in_progress = false;
                        wave_clear_timer = 0;
                        wave_cleared_prompt = true;
                        g_spawn_timer = 0;
                        g_last_wave = -1;
                        g_spawned_this_wave = 0;
                        g_max_in_this_wave = 0;
                    }
                    else
                    {
                        wave = sd.wave;
                        wave_in_progress = true;
                        wave_clear_timer = 0;
                    }
                    play_music("bgm", -1);
                    set_music_volume(1.0);
                    menu.in_menu = false;
                }
            }
            else if (act == MenuAction::Quit)
            {
                break;
            }
            continue; // skip gameplay updates while in menu
        }

        if (!shop_open)
        {
            if (key_typed(NUM_1_KEY) && weapons.size() > 0)
                current_weapon = 0;
            if (key_typed(NUM_2_KEY) && weapons.size() > 1)
                current_weapon = 1;
        }
        // In-game pause menu
        static PauseState pause;
        if (key_typed(ESCAPE_KEY) && !menu.in_menu)
        {
            if (shop_is_open(shop))
            {
                close_shop(shop);
            }
            else if (!pause.active)
            {
                init_pause(pause); pause_music();
            }
            else
            {
                pause.active = false; resume_music();
            }
        }

        if (player.alive)
        {
            // Wave clear handling: show prompt, allow Enter to start, B to open shop
            if (!wave_in_progress) wave_cleared_prompt = true;

            if (shop_is_open(shop))
            {
                bool should_close = update_shop(shop, weapons, current_weapon, money);
                if (should_close)
                {
                    close_shop(shop);
                    shop_open = false; // keep waiting for Enter
                    // no save here; save snapshot only at wave start
                }
            }

            if (!shop_is_open(shop))
            {
                // Open/close shop with B when not in combat
                if (key_typed(B_KEY) && !wave_in_progress)
                {
                    if (shop_open) close_shop(shop); else open_shop(shop);
                }

                // Start next wave on Enter when cleared
                if (wave < 5 && wave_cleared_prompt && key_typed(RETURN_KEY))
                {
                    wave_cleared_prompt = false;
                    wave++;
                    wave_in_progress = true;
                    wave_clear_timer = wave_clear_delay;
                    // Save snapshot at the start of this wave
                    SaveData sd; sd.money = money; sd.wave = wave; sd.slot_types[0] = -1; sd.slot_types[1] = -1; sd.current_slot = current_weapon;
                    for (int i = 0; i < 6 && i < (int)shop.items.size(); ++i) sd.unlocked[i] = shop.items[i].unlocked;
                    if (weapons.size() > 0 && weapons[0]) {
                        if (dynamic_cast<Pistol*>(weapons[0].get())) sd.slot_types[0] = 0;
                        else if (dynamic_cast<AK*>(weapons[0].get())) sd.slot_types[0] = 1;
                        else if (dynamic_cast<Shotgun*>(weapons[0].get())) sd.slot_types[0] = 2;
                        else if (dynamic_cast<AWP*>(weapons[0].get())) sd.slot_types[0] = 3;
                    }
                    if (weapons.size() > 1 && weapons[1]) {
                        if (dynamic_cast<Pistol*>(weapons[1].get())) sd.slot_types[1] = 0;
                        else if (dynamic_cast<AK*>(weapons[1].get())) sd.slot_types[1] = 1;
                        else if (dynamic_cast<Shotgun*>(weapons[1].get())) sd.slot_types[1] = 2;
                        else if (dynamic_cast<AWP*>(weapons[1].get())) sd.slot_types[1] = 3;
                    }
                    save_game(sd);
                }

                if (pause.active)
                {
                    PauseAction pa = update_pause(pause);
                    if (pa == PauseAction::Continue)
                    {
                        pause.active = false; resume_music();
                    }
                    else if (pa == PauseAction::Save)
                    {
                        // Disabled: only save at wave start
                        pause.saved_highlight = false;
                    }
                    else if (pa == PauseAction::MainMenu)
                    {
                        pause.active = false;
                        stop_music();
                        menu.in_menu = true; init_menu(menu, save_exists());
                        continue;
                    }
                    else if (pa == PauseAction::Quit)
                    {
                        return 0;
                    }

                    draw_pause_menu(pause, screen_w, screen_h);
                    refresh_screen(120);
                    continue;
                }

                // Update blocking inside player module
                update_player_block(player);

                if (!player.blocking)
                {
                    player.dash_disabled = false;
                    player.player_speed = 2.0;
                    update_player(player);
                }


                // Safe active weapon selection
                int active_idx = current_weapon;
                if (active_idx >= (int)weapons.size() || !weapons[active_idx])
                {
                    if (weapons.size() > 0 && weapons[0]) active_idx = 0;
                    else if (weapons.size() > 1 && weapons[1]) active_idx = 1;
                    else active_idx = -1;
                    if (active_idx != -1) current_weapon = active_idx;
                }
                if (active_idx != -1)
                {
                    weapons[active_idx]->update(player);
                    for (auto &e : enemies)
                        e->update(player, weapons[active_idx]->bullets());
                }

                if (wave_in_progress)
                {
                    spawn_enemies(enemies, wave, wave_in_progress);
                }
                // When not in progress, wait for Enter (no auto start)
            }
        }

        shop_open = shop_is_open(shop);

        double shake_x = 0, shake_y = 0;
        if (camera_shake_timer > 0 && !shop_open)
        {
            shake_x = rnd(-6, 6);
            shake_y = rnd(-6, 6);
            camera_shake_timer--;
        }

        draw_bitmap(background, shake_x, shake_y);

        if (!player.blocking)
        {
            if (player.damage_cooldown > 0)
            {
                if ((player.damage_cooldown / 5) % 2 == 0)
                    draw_player(player);
            }
            else
            {
                draw_player(player);
            }
        }

        // Draw block flash effect
        if (block_flash_timer > 0)
        {
            double cxp = player.player_x + player.player_width/2;
            double cyp = player.player_y + player.player_hight/2;
            color yel = COLOR_YELLOW; double len = 34;
            draw_line(yel, cxp - len, cyp, cxp + len, cyp);
            draw_line(yel, cxp, cyp - len, cxp, cyp + len);
            block_flash_timer--;
        }

        if (!player.blocking && !shop_open)
        {
            int active_idx = current_weapon;
            if (active_idx >= (int)weapons.size() || !weapons[active_idx])
            {
                if (weapons.size() > 0 && weapons[0]) active_idx = 0;
                else if (weapons.size() > 1 && weapons[1]) active_idx = 1;
                else active_idx = -1;
                if (active_idx != -1) current_weapon = active_idx;
            }
            if (active_idx != -1) weapons[active_idx]->draw(player);
        }

        if (!shop_open)
            for (auto &e : enemies)
                e->draw();

        // Boss HP bar (top)
        if (wave == 5)
        {
            for (auto &e : enemies)
            {
                if (!e->alive) continue;
                if (auto *b = dynamic_cast<Boss*>(e.get()))
                {
                    double ratio = (double)b->hp / (double)b->max_hp();
                    double bar_w = 800; double bar_h = 16;
                    double bx = screen_w/2 - bar_w/2; double by = 60;
                    color bar_color = b->enraged() ? COLOR_RED : COLOR_PURPLE;
                    fill_rectangle(bar_color, bx, by, bar_w * ratio, bar_h);
                    draw_rectangle(COLOR_BLACK, bx, by, bar_w, bar_h);
                    break;
                }
            }
        }

        // Draw block overlay when blocking
        draw_player_block_overlay(player);

        if (player.just_got_hit)
        {
            camera_shake_timer = 10;
            player.just_got_hit = false;
        }

        for (auto &c : coins)
        {
            if (!c.active)
                continue;
            draw_bitmap(c.frames[c.frame], c.x, c.y);
        }

        draw_player_hp(player);

        if (shop_open)
        {
            draw_shop(shop, weapons, money, screen_w);
        }

        if (!shop_open)
        {
            for (auto &c : coins)
            {
                if (!c.active)
                    continue;
                c.frame_timer++;
                if (c.frame_timer >= c.frame_interval)
                {
                    c.frame = (c.frame + 1) % c.frame_count;
                    c.frame_timer = 0;
                }
                double dx = player.player_x - c.x;
                double dy = player.player_y - c.y;
                double dist = sqrt(dx * dx + dy * dy);
                if (dist > 1)
                {
                    double suck_speed = 10.0; // slower coin speed
                    c.x += (dx / dist) * suck_speed;
                    c.y += (dy / dist) * suck_speed;
                }
                if (dist < 75)
                {
                    money += (int)c.value;
                    c.active = false;
                }
            }
        }

        // Draw coin UI to the right of HP hearts (6 hearts width)
        bitmap coin_ui = bitmap_named("coin_ui");
        double hp_block_w = 20 + 6*48; // left margin + 6 hearts
        double coin_x = hp_block_w + 40; double coin_y = 20;
        draw_bitmap(coin_ui, coin_x, coin_y);
        draw_text("x " + std::to_string(money), COLOR_BLACK, "arial", 32, coin_x + 50, coin_y + 5);

        if (!player.alive)
        {
            draw_text("GAME OVER!", COLOR_RED, "arial", 64, screen_w / 2 - 200, screen_h / 2 - 50);
            if (wave == 5)
                draw_text("Press 'R' to return to pre-boss intermission", COLOR_BLACK, "arial", 32, screen_w / 2 - 320, screen_h / 2 + 40);
            else
                draw_text("Press 'R' to Restart", COLOR_BLACK, "arial", 32, screen_w / 2 - 180, screen_h / 2 + 40);
            if (key_typed(R_KEY))
            {
                player.alive = true;
                player.knockback_dx = player.knockback_dy = 0;
                player.knockback_timer = 0;
                player.damage_cooldown = 0;
                player.just_got_hit = false;
                player.blocking = false; player.block_timer = 0; player.dash_disabled = false;
                player.player_x = screen_w / 2.0; player.player_y = screen_h / 2.0; player.player_speed = 2.0;

                enemies.clear();
                coins.clear();
                camera_shake_timer = 0;

                if (wave == 5)
                {
                    // Return to wave4 intermission (pre-boss)
                    if (player.max_hearts < 12) player.max_hearts += 1; // increase a heart up to 12
                    player.hearts = player.max_hearts;
                    wave = 4;
                    wave_in_progress = false;
                    wave_clear_timer = 0;
                    wave_cleared_prompt = true;
                    stop_music();
                    play_music("bgm", -1);
                    set_music_volume(1.0);
                    g_spawn_timer = 0;
                    g_last_wave = -1;
                    g_spawned_this_wave = 0;
                    g_max_in_this_wave = 0;
                }
                else
                {
                    // Normal restart to wave 1
                    weapons.clear();
                    auto ak2 = std::make_unique<AK>(); ak2->load_assets(); weapons.push_back(std::move(ak2));
                    auto pistol2 = std::make_unique<Pistol>(); pistol2->load_assets(); weapons.push_back(std::move(pistol2));
                    current_weapon = 0;
                    money = 1000;
                    wave = 1; wave_in_progress = true; wave_clear_timer = 0;
                    close_shop(shop); init_shop(shop);
                }
                continue;
            }
        }

        int alive_count = 0;
        for (auto &e : enemies)
            if (e->alive)
                alive_count++;
        int remaining_to_spawn = 0;
        if (wave_in_progress)
        {
            extern int g_max_in_this_wave;
            extern int g_spawned_this_wave;
            remaining_to_spawn = g_max_in_this_wave - g_spawned_this_wave;
            if (remaining_to_spawn < 0)
                remaining_to_spawn = 0;
        }
        int remaining_total = alive_count + remaining_to_spawn;
        // Boss wave / boss countdown HUD
        if (wave < 5)
        {
            int to_boss = 5 - wave;
            draw_text("Wave: " + std::to_string(wave) + "  Remaining: " + std::to_string(remaining_total) + "  Boss in " + std::to_string(to_boss) + " wave(s)",
                      COLOR_BLACK, "arial", 28, screen_w / 2 - 280, 10);
        }
        else if (wave == 5)
        {
            draw_text("BOSS FIGHT", COLOR_RED, "arial", 36, screen_w / 2 - 120, 10);
        }

        static double spread = 10;
        const double line_len = 8;
        double cx = mouse_x(), cy = mouse_y();
        color cross_color = COLOR_GREEN;
        if (mouse_down(LEFT_BUTTON))
        {
            spread += 0.6;
            if (spread > 20)
                spread = 20;
        }
        else
        {
            spread -= 0.4;
            if (spread < 10)
                spread = 10;
        }
        draw_line(cross_color, cx - spread - line_len, cy, cx - spread, cy);
        draw_line(cross_color, cx + spread, cy, cx + spread + line_len, cy);
        draw_line(cross_color, cx, cy - spread - line_len, cx, cy - spread);
        draw_line(cross_color, cx, cy + spread, cx, cy + spread + line_len);

        // Kill hitmarker: draw a red version of the crosshair rotated 45°
        if (kill_marker_timer > 0)
        {
            color kc = COLOR_RED;
            double inv = 0.70710678; // 1/sqrt(2)
            // unit vectors along the diagonals
            double ux1 =  inv, uy1 =  inv; // down-right
            double ux2 =  inv, uy2 = -inv; // up-right
            double d = spread;
            double l = line_len;
            // four segments at +/-d along diagonals, length l going outward
            // +diag1
            double x1 = cx + ux1 * d, y1 = cy + uy1 * d;
            draw_line(kc, x1, y1, x1 + ux1 * l, y1 + uy1 * l);
            // -diag1
            double x2 = cx - ux1 * d, y2 = cy - uy1 * d;
            draw_line(kc, x2, y2, x2 - ux1 * l, y2 - uy1 * l);
            // +diag2
            double x3 = cx + ux2 * d, y3 = cy + uy2 * d;
            draw_line(kc, x3, y3, x3 + ux2 * l, y3 + uy2 * l);
            // -diag2
            double x4 = cx - ux2 * d, y4 = cy - uy2 * d;
            draw_line(kc, x4, y4, x4 - ux2 * l, y4 - uy2 * l);
            kill_marker_timer--;
        }

        // Wave cleared prompt text (center screen)
        if (!wave_in_progress)
        {
            if (wave == 5)
            {
                std::string l = "BOSS defeated! Press \"Enter\" to return to Main Menu";
                double s = 26; double w = l.size()*s*0.6; double x = screen_w/2 - w/2; double y = screen_h/2 - 20;
                draw_text(l, COLOR_YELLOW, "arial", (int)s, x, y);
                if (key_typed(RETURN_KEY)) { menu.in_menu = true; init_menu(menu, save_exists()); stop_music(); delete_save(); }
            }
            else
            {
                std::string l1 = "Wave cleared! Press \"Enter\" to start the next wave.";
                std::string l2 = "Press \"B\" to open the shop";
                double s1 = 26, s2 = 24; double w1 = l1.size()*s1*0.6; double w2 = l2.size()*s2*0.6; double x1 = screen_w/2 - w1/2; double x2 = screen_w/2 - w2/2; double y = screen_h/2 - 20;
                draw_text(l1, COLOR_YELLOW, "arial", (int)s1, x1, y - 24);
                draw_text(l2, COLOR_WHITE,  "arial", (int)s2, x2, y + 8);
            }
        }

        refresh_screen(120);

        if (!shop_open)
            shop.mouse_prev_down = mouse_down(LEFT_BUTTON);
    }
    return 0;
}
