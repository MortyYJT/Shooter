// Hilichurl Archer: keeps distance, loads, then fires slow inaccurate projectiles.
#pragma once

#include "../enemy_base.hpp"

// Hilichurl Archer: keeps distance and fires after loading
class HilichurlArcher : public EnemyBase
{
public:
    HilichurlArcher() {}

    void load_assets() override;
    void update(player_data &player, std::vector<Bullet> &bullets) override;
    void draw() const override;

private:
    // Visuals
    bitmap unloaded_{nullptr};
    bitmap loaded_{nullptr};
    bool is_loaded_ = false;
    bool facing_left_ = false;

    // Movement
    double speed_ = 0.4;
    const double prefer_dist_ = 300.0;
    const double min_dist_ = 240.0;

    // Loading / firing
    int reload_timer_ = 0;
    const int reload_time_ = 150; // slower fire rate
    int loaded_display_timer_ = 0;
    const int loaded_display_time_ = 20;

    // Simple enemy projectile (arrow) container
    struct Arrow { double x,y,dx,dy; bool active; };
    std::vector<Arrow> arrows_;
};
