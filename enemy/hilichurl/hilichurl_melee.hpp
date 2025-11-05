// Hilichurl (melee): chases player, telegraphs with a yellow cross, then performs
// a 3-frame attack. Player can block; on successful block, Hilichurl enters a long
// recovery. Damage is applied only during the core hit frame.
#pragma once

#include "../enemy_base.hpp"

// Hilichurl (melee) with telegraph + 3-frame attack
class HilichurlMelee : public EnemyBase
{
public:
    HilichurlMelee() {}

    void load_assets() override;
    void update(player_data &player, std::vector<Bullet> &bullets) override;
    void draw() const override;

private:
    enum State
    {
        CHASE,
        TELEGRAPH,
        ATTACK,
        RECOVER
    } state_ = CHASE;

    // Visuals
    bitmap idle_{nullptr};
    bitmap attack_frames_[3]{};
    bool facing_left_ = false;

    // Movement & sizing
    double speed_ = 0.7;

    // Telegraph
    int telegraph_timer_ = 0;
    const int telegraph_duration_ = 30; // yellow flash duration (extended)

    // Attack anim
    int atk_frame_ = 0;
    int atk_timer_ = 0;
    const int atk_interval_ = 6; // frames per attack frame
    bool dealt_damage_ = false;  // ensure single hit
    bool was_blocked_ = false;   // track if last attack was blocked

    // Recover
    int recover_timer_ = 0;
    const int recover_duration_ = 180;           // normal recover also slowed
    const int blocked_recover_duration_ = 180;   // ~1.5s at 120 FPS
};
