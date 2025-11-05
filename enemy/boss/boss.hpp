#pragma once
#include "../enemy_base.hpp"
#include <vector>

class Boss : public EnemyBase
{
public:
    Boss();
    void load_assets() override;
    void update(player_data &player, std::vector<Bullet> &bullets) override;
    void draw() const override;

    int max_hp() const { return phase_ == 1 ? max_hp_phase1_ : max_hp_phase2_; }
    bool enraged() const { return phase_ == 2; }

private:
    enum class State
    {
        Intro,
        P1_Fan,
        P1_Rest,
        PhaseHalfCue,
        LowHpCue,
        Phase1Death,
        Rebirth,
        P2_Fan,
        P2_Rest,
        P2_LasersGrow,
        P2_Lasers,
        PlayerLose,
        DeadFinal
    };

    int phase_ = 1;
    State state_ = State::Intro;
    State state_before_cue_ = State::P1_Fan;
    int state_timer_ = 0;
    int state_limit_ = 0;
    int lasers_timer_ = 0;
    double base_speed_ = 0.75;
    double vx_ = 0.0;
    double vy_ = 0.0;
    bool half_cue_played_ = false;
    bool lowhp_cue_played_ = false;
    bool phase1_finished_ = false;
    bool initial_p1_rest_done_ = false;
    bool bgm_stage1_playing_ = false;
    bool bgm_stage2_playing_ = false;
    bool stage2_bgm_pending_ = false;
    int rebirth_audio_timer_ = 0;


    bool player_lose_followup_pending_ = false;
    int sfx_timer_ = 0;
    double fan_angle_offset_ = 0.0;
    int fan_move_timer_ = 0;
    double lasers_current_length_ = 0.0;
    double lasers_inner_radius_ = 0.0;
    int lasers_grow_timer_ = 0;

    int max_hp_phase1_ = 12000;
    int max_hp_phase2_ = static_cast<int>(12000 * 1.5);
    int contact_cooldown_ = 0;

    bitmap img_regular_{nullptr};
    bitmap img_half0_{nullptr};
    bitmap img_half1_{nullptr};
    bitmap img_lowhp_{nullptr};
    bitmap img_dead_{nullptr};
    bitmap img_rebirth_{nullptr};
    bitmap img_player_lose_{nullptr};
    bitmap img_bullet_fan_{nullptr};

    struct BossBullet
    {
        double x;
        double y;
        double dx;
        double dy;
        int life;
        bool active;
    };

    struct Laser
    {
        double ang;
    };

    std::vector<BossBullet> shots_;
    std::vector<Laser> lasers_;

    void enter_state(State new_state);
    bool is_invulnerable_state() const;
    void play_stage1_bgm(bool force_restart = false);
    void play_stage2_bgm(bool force_restart = false);
    void stop_active_sfx();
    void fire_fan(int count, double speed);

    void update_intro(player_data &player);
    void update_p1_fan();
    void update_p1_rest(player_data &player);
    void update_phase_half_cue();
    void update_low_hp_cue();
    void update_phase1_death();
    void update_rebirth();
    void update_p2_fan();
    void update_p2_rest(player_data &player);
    void update_p2_lasers_grow(player_data &player);
    void update_p2_lasers(player_data &player);
    void update_player_lose();

    void deal_damage_to_player(player_data &player);
    void handle_player_collision(player_data &player);
    void handle_player_bullets(std::vector<Bullet> &bullets);
    void update_projectiles(player_data &player);
    void update_lasers_damage(player_data &player);
    void reset_position_to_center();
};
