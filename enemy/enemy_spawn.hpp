#pragma once
#include <vector>
#include <memory>
#include "enemy_base.hpp"
#include "slime/slime.hpp"

// Wave spawn tracking (exposed for HUD)
inline int g_spawn_timer = 0;
inline int g_last_wave = -1;
inline int g_spawned_this_wave = 0;
inline int g_max_in_this_wave = 0;
#include "hilichurl/hilichurl_melee.hpp"
#include "hilichurl/hilichurl_archer.hpp"
#include "boss/boss.hpp"

// Enemy spawning logic (simple waves)
inline void spawn_enemies(std::vector<std::unique_ptr<EnemyBase>> &enemies,
                          int wave,
                          bool &wave_in_progress)
{
    // State (preserves info across frames)
    int &spawn_timer = g_spawn_timer;
    int &last_wave = g_last_wave;
    int &spawned_this_wave = g_spawned_this_wave;

    // Parameter configuration
    const int spawn_interval = 60;              // Attempt to spawn an enemy every 60 frames
    const int max_in_screen = 10;               // Max enemies on screen simultaneously
    int max_in_this_wave = (wave==5 ? 1 : 10 + (wave - 1) * 8); // Wave 5: Boss only

    if (max_in_this_wave > 200)
        max_in_this_wave = 200;
    g_max_in_this_wave = max_in_this_wave;

    // Reset when new wave starts
    if (wave != last_wave)
    {
        last_wave = wave;
        spawn_timer = 0;
        spawned_this_wave = 0;
    }

    // Count currently alive enemies
    int alive_count = 0;
    for (auto &e : enemies)
        if (e->alive)
            alive_count++;

    // Check if wave is complete
    bool all_spawned = (spawned_this_wave >= max_in_this_wave);
    if (all_spawned && alive_count == 0)
    {
        wave_in_progress = false; // Current wave ends
        return;
    }

    // Special Boss wave: spawn immediately and only once; finish when dead
    if (wave == 5)
    {
        if (spawned_this_wave == 0)
        {
            auto boss = std::make_unique<Boss>();
            boss->load_assets();
            boss->x = screen_width() / 2.0 - boss->width / 2.0;
            boss->y = -boss->height - 40.0;
            enemies.push_back(std::move(boss));
            spawned_this_wave = 1;
        }
        // Check completion
        int alive_boss = 0; for (auto &e : enemies) if (e->alive) alive_boss++;
        if (alive_boss == 0) { wave_in_progress = false; }
        return;
    }

    // Control spawn timing
    spawn_timer++;
    if (spawn_timer < spawn_interval)
        return;      // Interval not reached
    spawn_timer = 0; // Reset timer

    if (alive_count >= max_in_screen)
        return; // Too many on screen, don't spawn
    if (spawned_this_wave >= max_in_this_wave)
        return; // Already spawned enough

    // Randomly determine enemy type to spawn by wave
    int type = rnd(100); // 0..99

    std::unique_ptr<EnemyBase> enemy;
    if (wave <= 1)
    {
        // Wave 1: Slime 60%, Melee 40%
        if (type < 60) enemy = std::make_unique<SlimeEnemy>();
        else enemy = std::make_unique<HilichurlMelee>();
    }
    else
    {
        // Wave 2+: Slime 55%, Melee 35%, Archer 10% (reduce archers)
        if (type < 55) enemy = std::make_unique<SlimeEnemy>();
        else if (type < 90) enemy = std::make_unique<HilichurlMelee>();
        else enemy = std::make_unique<HilichurlArcher>();
    }

    enemy->load_assets();

    // Random spawn outside screen edges
    double w = screen_width();
    double h = screen_height();
    double x, y;
    int side = rnd(4); // 0=top, 1=right, 2=bottom, 3=left

    switch (side)
    {
    case 0:
        x = rnd(w - 48);
        y = -48;
        break;
    case 1:
        x = w + 48;
        y = rnd(h - 48);
        break;
    case 2:
        x = rnd(w - 48);
        y = h + 48;
        break;
    case 3:
        x = -48;
        y = rnd(h - 48);
        break;
    }

    enemy->x = x;
    enemy->y = y;

    enemies.push_back(std::move(enemy));
    spawned_this_wave++;
}
