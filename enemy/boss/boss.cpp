#include "boss.hpp"
#include <cmath>
#include <algorithm>

extern int block_flash_timer;

namespace
{
    constexpr double PI = 3.141592654;
    constexpr int FPS = 120;

    constexpr int FAN_INTERVAL_FRAMES = 16;    // ~0.13s
    constexpr int P1_REST_MIN = 220;          // ~1.8s
    constexpr int P1_REST_MAX = 260;          // ~2.2s
    constexpr int P2_REST_MIN = 220;          // ~1.8s
    constexpr int P2_REST_MAX = 260;          // ~2.2s

    constexpr int SFX_HIT_PLAYER_FRAMES  = 240;   // 00:00:02
    constexpr int SFX_DEAD_FRAMES        = 120;   // 00:00:01
    constexpr int SFX_HALF_FRAMES        = 360;   // 00:00:03
    constexpr int SFX_LOWHP_FRAMES       = 120;   // 00:00:01
    constexpr int SFX_PLAYERLOSE_FRAMES  = 120;   // 00:00:01
    constexpr int REBIRTH_DISPLAY_FRAMES = 300;   // ~2.5s visual
    constexpr int REBIRTH_AUDIO_FRAMES   = 600;   // ~5s audio
    constexpr int LASERS_DURATION_FRAMES = 10 * FPS;
    constexpr int LASER_GROW_FRAMES = 5 * FPS;
    constexpr double LASER_ROTATE_RATE = 0.0008;
    constexpr double LASER_MAX_LENGTH = 1200.0;

    inline int rand_range(int min_val, int max_val)
    {
        if (max_val <= min_val) return min_val;
        return min_val + rnd(max_val - min_val + 1);
    }

    inline double clamp(double v, double lo, double hi)
    {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

    inline bool point_in_rect(double px, double py, double rx, double ry, double rw, double rh)
    {
        return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
    }

    inline bool segment_intersects_rect(double x1, double y1, double x2, double y2,
                                        double rx, double ry, double rw, double rh)
    {
        if (point_in_rect(x1, y1, rx, ry, rw, rh) || point_in_rect(x2, y2, rx, ry, rw, rh))
            return true;

        auto cross = [](double ux, double uy, double vx, double vy) { return ux * vy - uy * vx; };

        auto seg_intersect = [&](double ax, double ay, double bx, double by,
                                 double cx, double cy, double dx, double dy) {
            double abx = bx - ax, aby = by - ay;
            double acx = cx - ax, acy = cy - ay;
            double adx = dx - ax, ady = dy - ay;
            double cdx = dx - cx, cdy = dy - cy;
            double cax = ax - cx, cay = ay - cy;

            double d1 = cross(abx, aby, acx, acy);
            double d2 = cross(abx, aby, adx, ady);
            double d3 = cross(cdx, cdy, cax, cay);
            double d4 = cross(cdx, cdy, bx - cx, by - cy);

            return (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
                    ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0)));
        };

        double left   = rx;
        double right  = rx + rw;
        double top    = ry;
        double bottom = ry + rh;

        if (seg_intersect(x1, y1, x2, y2, left,  top, left,  bottom)) return true;
        if (seg_intersect(x1, y1, x2, y2, right, top, right, bottom)) return true;
        if (seg_intersect(x1, y1, x2, y2, left,  top, right, top))    return true;
        if (seg_intersect(x1, y1, x2, y2, left,  bottom, right, bottom)) return true;
        return false;
    }
}

Boss::Boss()
{
    width = 200;
    height = 200;
    hp = max_hp_phase1_;
}

void Boss::load_assets()
{
    load_bitmap("boss_regular", "../image/enemy/BOSS/regular.png");
    load_bitmap("boss_half0",    "../image/enemy/BOSS/half_life_0.png");
    load_bitmap("boss_half1",    "../image/enemy/BOSS/half_life_1.png");
    load_bitmap("boss_lowhp",    "../image/enemy/BOSS/low_hp.png");
    load_bitmap("boss_dead",     "../image/enemy/BOSS/stage_1_dead.png");
    load_bitmap("boss_rebirth",  "../image/enemy/BOSS/rebirth.png");
    load_bitmap("boss_plose",    "../image/enemy/BOSS/player_lose.png");
    load_bitmap("bullet_alt2_1", "../image/weapon/Bullet_Alt2_1.png");

    img_regular_     = bitmap_named("boss_regular");
    img_half0_       = bitmap_named("boss_half0");
    img_half1_       = bitmap_named("boss_half1");
    img_lowhp_       = bitmap_named("boss_lowhp");
    img_dead_        = bitmap_named("boss_dead");
    img_rebirth_     = bitmap_named("boss_rebirth");
    img_player_lose_ = bitmap_named("boss_plose");
    img_bullet_fan_  = bitmap_named("bullet_alt2_1");

    enter_state(State::Intro);
}

void Boss::play_stage1_bgm(bool force_restart)
{
    if (force_restart)
    {
        stop_music();
        bgm_stage1_playing_ = false;
        bgm_stage2_playing_ = false;
    }

    if (!bgm_stage1_playing_)
    {
        load_music("stage1", "../sound/bgm/stage_1_bgm.mp3");
        play_music("stage1", -1);
        set_music_volume(0.5);
        bgm_stage1_playing_ = true;
        bgm_stage2_playing_ = false;
    }
}

void Boss::play_stage2_bgm(bool force_restart)
{
    if (!force_restart && stage2_bgm_pending_)
        return;

    if (force_restart)
    {
        stop_music();
        bgm_stage1_playing_ = false;
        bgm_stage2_playing_ = false;
    }

    if (!bgm_stage2_playing_)
    {
        load_music("stage2", "../sound/bgm/stage_2_bgm.mp3");
        play_music("stage2", -1);
        set_music_volume(1.0);
        bgm_stage2_playing_ = true;
        bgm_stage1_playing_ = false;
    }
}

void Boss::stop_active_sfx()
{
    const char* names[] = {
        "boss_hit_player",
        "boss_dead",
        "boss_half",
        "boss_lowhp",
        "boss_playerlose",
        "boss_rebirth",
        "block_sfx"
    };

    for (const char* name : names)
    {
        sound_effect fx = sound_effect_named(name);
        if (fx)
            stop_sound_effect(fx);
    }
}

static void play_sfx_boosted(const std::string &name)
{
    sound_effect fx = sound_effect_named(name);
    if (!fx)
        return;

    // Play multiple overlapping instances to approximate a 3.5× gain.
    play_sound_effect(fx, 1.0);
    play_sound_effect(fx, 1.0);
    play_sound_effect(fx, 1.0);
    play_sound_effect(fx, 0.5);
}

void Boss::enter_state(State new_state)
{
    state_ = new_state;
    state_timer_ = 0;
    state_limit_ = 0;

    switch (state_)
    {
    case State::Intro:
        shots_.clear();
        lasers_.clear();
        reset_position_to_center();
        y = -height - 40;
        vx_ = vy_ = 0.0;
        initial_p1_rest_done_ = false;
        fan_angle_offset_ = 0.0;
        break;

    case State::P1_Fan:
        state_limit_ = 10 * FPS;
        shots_.clear();
        vx_ = vy_ = 0.0;
        fan_angle_offset_ = rnd(360) * PI / 180.0;
        break;

    case State::P1_Rest:
        if (!initial_p1_rest_done_)
        {
            state_limit_ = 2 * FPS;
            initial_p1_rest_done_ = true;
        }
        else
        {
            state_limit_ = rand_range(P1_REST_MIN, P1_REST_MAX);
        }
        break;

    case State::PhaseHalfCue:
        state_limit_ = SFX_HALF_FRAMES;
        load_sound_effect("boss_half", "../sound/BOSS/half_life.mp3");
        play_sfx_boosted("boss_half");
        break;

    case State::LowHpCue:
        state_limit_ = SFX_LOWHP_FRAMES;
        load_sound_effect("boss_lowhp", "../sound/BOSS/low_hp.mp3");
        play_sfx_boosted("boss_lowhp");
        break;

    case State::Phase1Death:
        state_limit_ = SFX_DEAD_FRAMES;
        load_sound_effect("boss_dead", "../sound/BOSS/dead.mp3");
        play_sfx_boosted("boss_dead");
        shots_.clear();
        break;

    case State::Rebirth:
        stop_active_sfx();
        stop_music();
        state_limit_ = REBIRTH_DISPLAY_FRAMES;
        stage2_bgm_pending_ = true;
        rebirth_audio_timer_ = REBIRTH_AUDIO_FRAMES;
        bgm_stage1_playing_ = false;
        bgm_stage2_playing_ = false;
        sfx_timer_ = REBIRTH_AUDIO_FRAMES;
        load_sound_effect("boss_rebirth", "../sound/BOSS/rebirth.mp3");
        play_sfx_boosted("boss_rebirth");
        reset_position_to_center();
        shots_.clear();
        lasers_.clear();
        fan_angle_offset_ = 0.0;
        break;

    case State::P2_Fan:
        state_limit_ = 10 * FPS;
        shots_.clear();
        vx_ = vy_ = 0.0;
        fan_angle_offset_ = rnd(360) * PI / 180.0;
        break;

    case State::P2_Rest:
        state_limit_ = rand_range(P2_REST_MIN, P2_REST_MAX);
        break;

    case State::P2_LasersGrow:
        lasers_.clear();
        lasers_timer_ = 0;
        lasers_grow_timer_ = LASER_GROW_FRAMES;
        lasers_inner_radius_ = std::max(width, height) / 2.0 + 10.0;
        lasers_current_length_ = lasers_inner_radius_;
        for (int i = 0; i < 8; ++i)
            lasers_.push_back({ i * (PI / 4.0) });
        reset_position_to_center();
        break;

    case State::P2_Lasers:
        lasers_timer_ = LASERS_DURATION_FRAMES;
        lasers_inner_radius_ = std::max(width, height) / 2.0 + 10.0;
        lasers_current_length_ = LASER_MAX_LENGTH;
        reset_position_to_center();
        break;

    case State::PlayerLose:
        load_sound_effect("boss_playerlose", "../sound/BOSS/player_lose.mp3");
        play_sfx_boosted("boss_playerlose");
        sfx_timer_ = SFX_PLAYERLOSE_FRAMES;
        player_lose_followup_pending_ = true;
        vx_ = vy_ = 0.0;
        break;

    case State::DeadFinal:
        shots_.clear();
        lasers_.clear();
        vx_ = vy_ = 0.0;
        break;
    }
}

bool Boss::is_invulnerable_state() const
{
    switch (state_)
    {
    case State::Intro:
    case State::PhaseHalfCue:
    case State::LowHpCue:
    case State::Phase1Death:
    case State::Rebirth:
    case State::PlayerLose:
    case State::DeadFinal:
        return true;
    default:
        return false;
    }
}

void Boss::fire_fan(int count, double speed)
{
    double cx = x + width / 2.0;
    double cy = y + height / 2.0;
    double base = fan_angle_offset_;
    for (int i = 0; i < count; ++i)
    {
        double ang = base + i * (2.0 * PI / count);
        BossBullet shot{ cx, cy, std::cos(ang) * speed, std::sin(ang) * speed, -1, true };
        shots_.push_back(shot);
    }

    double delta = 10.0 * PI / 180.0;
    fan_angle_offset_ += delta;
}

void Boss::update_intro(player_data &player)
{
    play_stage1_bgm();

    double target_x = screen_width() / 2.0 - width / 2.0;
    double target_y = screen_height() / 2.0 - height / 2.0;
    double speed    = 5.0;

    if (std::fabs(x - target_x) > 1.0)
        x += (x < target_x ? speed : -speed);
    if (std::fabs(y - target_y) > 1.0)
        y += (y < target_y ? speed : -speed);

    if (std::fabs(x - target_x) <= 1.0 && std::fabs(y - target_y) <= 1.0)
    {
        if (state_timer_ == 0)
        {
            load_sound_effect("boss_hit_player", "../sound/BOSS/hit_player.mp3");
            play_sfx_boosted("boss_hit_player");
        }

        if (++state_timer_ >= SFX_HIT_PLAYER_FRAMES)
        {
            enter_state(State::P1_Fan);
        }
    }
}

void Boss::update_p1_fan()
{
    play_stage1_bgm();
    if (fan_move_timer_ <= 0)
    {
        double dx = rand_range(-60, 60);
        double dy = rand_range(-60, 60);
        x = clamp(x + dx, 0.0, screen_width() - width);
        y = clamp(y + dy, 0.0, screen_height() - height);
        fan_move_timer_ = rand_range(FPS - 20, FPS + 20);
    }
    else
    {
        fan_move_timer_--;
    }

    if (state_timer_ < state_limit_ && state_timer_ % FAN_INTERVAL_FRAMES == 0)
    {
        fire_fan(8, 1.4);
    }

    if (++state_timer_ >= state_limit_)
    {
        enter_state(State::P1_Rest);
    }
}

void Boss::update_p1_rest(player_data &player)
{
    double target_x = player.player_x - width / 2.0;
    double target_y = player.player_y - height / 2.0;
    double dx = target_x - x;
    double dy = target_y - y;
    double dist = std::sqrt(dx * dx + dy * dy);
    if (dist > 1.0)
    {
        x += (dx / dist) * base_speed_;
        y += (dy / dist) * base_speed_;
    }

    if (++state_timer_ >= state_limit_)
    {
        enter_state(State::P1_Fan);
    }
}

void Boss::update_phase_half_cue()
{
    if (++state_timer_ >= state_limit_)
    {
        half_cue_played_ = true;
        enter_state(state_before_cue_);
    }
}

void Boss::update_low_hp_cue()
{
    if (++state_timer_ >= state_limit_)
    {
        lowhp_cue_played_ = true;
        enter_state(state_before_cue_);
    }
}

void Boss::update_phase1_death()
{
    if (++state_timer_ >= state_limit_)
    {
        phase1_finished_ = true;
        enter_state(State::Rebirth);
    }
}

void Boss::update_rebirth()
{
    if (++state_timer_ >= state_limit_)
    {
        phase_ = 2;
        hp = max_hp_phase2_;
        enter_state(State::P2_Fan);
    }
}

void Boss::update_p2_fan()
{
    play_stage2_bgm();
    if (fan_move_timer_ <= 0)
    {
        double dx = rand_range(-60, 60);
        double dy = rand_range(-60, 60);
        x = clamp(x + dx, 0.0, screen_width() - width);
        y = clamp(y + dy, 0.0, screen_height() - height);
        fan_move_timer_ = rand_range(FPS - 20, FPS + 20);
    }
    else
    {
        fan_move_timer_--;
    }

    if (state_timer_ < state_limit_ && state_timer_ % FAN_INTERVAL_FRAMES == 0)
    {
        fire_fan(8, 1.82);
    }

    if (++state_timer_ >= state_limit_)
    {
        if (rnd(100) < 50)
            enter_state(State::P2_LasersGrow);
        else
            enter_state(State::P2_Rest);
    }
}

void Boss::update_p2_rest(player_data &player)
{
    play_stage2_bgm();
    double target_x = player.player_x - width / 2.0;
    double target_y = player.player_y - height / 2.0;
    double dx = target_x - x;
    double dy = target_y - y;
    double dist = std::sqrt(dx * dx + dy * dy);
    if (dist > 1.0)
    {
        x += (dx / dist) * base_speed_;
        y += (dy / dist) * base_speed_;
    }

    if (++state_timer_ >= state_limit_)
    {
        enter_state(State::P2_Fan);
    }
}

void Boss::update_p2_lasers_grow(player_data &player)
{
    play_stage2_bgm();
    x += (screen_width() / 2.0 - width / 2.0 - x) * 0.12;
    y += (screen_height() / 2.0 - height / 2.0 - y) * 0.12;

    double growth_per = (LASER_MAX_LENGTH - lasers_inner_radius_) / LASER_GROW_FRAMES;
    lasers_current_length_ = std::min(LASER_MAX_LENGTH, lasers_current_length_ + growth_per);

    update_lasers_damage(player);

    if (--lasers_grow_timer_ <= 0)
    {
        enter_state(State::P2_Lasers);
        return;
    }
}

void Boss::update_p2_lasers(player_data &player)
{
    play_stage2_bgm();
    lasers_current_length_ = LASER_MAX_LENGTH;
    x += (screen_width() / 2.0 - width / 2.0 - x) * 0.12;
    y += (screen_height() / 2.0 - height / 2.0 - y) * 0.12;

    for (auto &laser : lasers_)
        laser.ang += LASER_ROTATE_RATE;

    update_lasers_damage(player);

    if (--lasers_timer_ <= 0)
    {
        enter_state(State::P2_Rest);
    }
}

void Boss::update_player_lose()
{
    if (player_lose_followup_pending_ && --sfx_timer_ <= 0)
    {
        load_sound_effect("boss_hit_player", "../sound/BOSS/hit_player.mp3");
        play_sfx_boosted("boss_hit_player");
        player_lose_followup_pending_ = false;
    }
}

void Boss::deal_damage_to_player(player_data &player)
{
    if (player.damage_cooldown > 0 || player.blocking)
        return;

    if (player.hearts > 1)
    {
        load_sound_effect("boss_hit_player", "../sound/BOSS/hit_player.mp3");
        play_sfx_boosted("boss_hit_player");
    }

    player.hearts -= 1;
    if (player.hearts < 0) player.hearts = 0;
    player.just_got_hit = true;
    player.damage_cooldown = player.damage_cooldown_max;

    if (player.hearts == 0)
    {
        player.alive = false;
        enter_state(State::PlayerLose);
    }
}

void Boss::handle_player_collision(player_data &player)
{
    if (contact_cooldown_ > 0)
    {
        --contact_cooldown_;
        return;
    }

    double px = player.player_x;
    double py = player.player_y;
    double pw = player.player_width;
    double ph = player.player_hight;

    bool overlap = (px < x + width && px + pw > x && py < y + height && py + ph > y);
    if (!overlap)
        return;

    if (player.blocking)
    {
        load_sound_effect("block_sfx", "../sound/block.mp3");
        play_sfx_boosted("block_sfx");
        block_flash_timer = 8;
    }
    else
    {
        deal_damage_to_player(player);

        double cx = x + width / 2.0;
        double cy = y + height / 2.0;
        double dx = player.player_x + pw / 2.0 - cx;
        double dy = player.player_y + ph / 2.0 - cy;
        double len = std::sqrt(dx * dx + dy * dy);
        if (len > 0.0)
        {
            dx /= len;
            dy /= len;
            player.knockback_dx = dx * 9.0;
            player.knockback_dy = dy * 9.0;
            player.knockback_timer = 6;
        }
    }

    contact_cooldown_ = 30;
}

void Boss::handle_player_bullets(std::vector<Bullet> &bullets)
{
    if (is_invulnerable_state())
        return;

    for (auto &b : bullets)
    {
        if (!b.active)
            continue;

        double prev_x = b.x - b.dx;
        double prev_y = b.y - b.dy;
        bool hit = point_in_rect(b.x, b.y, x, y, width, height) ||
                   segment_intersects_rect(prev_x, prev_y, b.x, b.y, x, y, width, height);
        if (!hit)
            continue;

        hp -= b.damage;
        if (!b.piercing) b.active = false;

        if (phase_ == 1 && !half_cue_played_ && hp <= max_hp_phase1_ / 2)
        {
            state_before_cue_ = State::P1_Fan;
            enter_state(State::PhaseHalfCue);
            return;
        }

        if (phase_ == 1 && !lowhp_cue_played_ && hp <= static_cast<int>(max_hp_phase1_ * 0.15))
        {
            state_before_cue_ = State::P1_Fan;
            enter_state(State::LowHpCue);
            return;
        }

        if (phase_ == 1 && hp <= 0 && !phase1_finished_)
        {
            hp = 0;
            enter_state(State::Phase1Death);
            return;
        }

        if (phase_ == 2 && hp <= 0)
        {
            hp = 0;
            alive = false;
            enter_state(State::DeadFinal);
            return;
        }
    }
}

void Boss::update_projectiles(player_data &player)
{
    for (auto &shot : shots_)
    {
        if (!shot.active) continue;

        shot.x += shot.dx;
        shot.y += shot.dy;
        if (shot.life > 0)
        {
            if (--shot.life <= 0)
            {
                shot.active = false;
                continue;
            }
        }

        if (!shot.active) continue;

        double px = player.player_x + player.player_width / 2.0;
        double py = player.player_y + player.player_hight / 2.0;
        if (std::fabs(px - shot.x) < player.player_width / 2.0 &&
            std::fabs(py - shot.y) < player.player_hight / 2.0)
        {
            if (player.blocking)
            {
                load_sound_effect("block_sfx", "../sound/block.mp3");
                play_sfx_boosted("block_sfx");
                block_flash_timer = 8;
            }
            else
            {
                deal_damage_to_player(player);
            }
            shot.active = false;
        }
    }

    std::vector<BossBullet> alive;
    alive.reserve(shots_.size());
    for (auto &shot : shots_) if (shot.active) alive.push_back(shot);
    shots_ = std::move(alive);
}

void Boss::update_lasers_damage(player_data &player)
{
    double cx = x + width / 2.0;
    double cy = y + height / 2.0;
    double px = player.player_x + player.player_width / 2.0;
    double py = player.player_y + player.player_hight / 2.0;

    double inner = lasers_inner_radius_;
    double max_len = (state_ == State::P2_LasersGrow) ? lasers_current_length_ : LASER_MAX_LENGTH;

    for (const auto &laser : lasers_)
    {
        double dirx = std::cos(laser.ang);
        double diry = std::sin(laser.ang);
        double proj = (px - cx) * dirx + (py - cy) * diry;
        if (proj <= inner || proj > max_len)
            continue;

        double nx = std::sin(laser.ang);
        double ny = -std::cos(laser.ang);
        double dist = std::fabs((px - cx) * nx + (py - cy) * ny);
        if (dist < 14.0)
        {
            if (player.blocking)
            {
                load_sound_effect("block_sfx", "../sound/block.mp3");
                play_sfx_boosted("block_sfx");
                block_flash_timer = 8;
            }
            else
            {
                deal_damage_to_player(player);
            }
            return;
        }
    }
}

void Boss::reset_position_to_center()
{
    x = screen_width() / 2.0 - width / 2.0;
    y = screen_height() / 2.0 - height / 2.0;
}

void Boss::update(player_data &player, std::vector<Bullet> &bullets)
{
    switch (state_)
    {
    case State::Intro:           update_intro(player);           break;
    case State::P1_Fan:          update_p1_fan();                break;
    case State::P1_Rest:         update_p1_rest(player);         break;
    case State::PhaseHalfCue:    update_phase_half_cue();        break;
    case State::LowHpCue:        update_low_hp_cue();            break;
    case State::Phase1Death:     update_phase1_death();          break;
    case State::Rebirth:         update_rebirth();               break;
    case State::P2_Fan:          update_p2_fan();                break;
    case State::P2_Rest:         update_p2_rest(player);         break;
    case State::P2_LasersGrow:   update_p2_lasers_grow(player);  break;
    case State::P2_Lasers:       update_p2_lasers(player);       break;
    case State::PlayerLose:      update_player_lose();           break;
    case State::DeadFinal:                                       break;
    }

    handle_player_collision(player);
    handle_player_bullets(bullets);
    update_projectiles(player);

    if (stage2_bgm_pending_)
    {
        if (rebirth_audio_timer_ > 0)
            --rebirth_audio_timer_;
        if (rebirth_audio_timer_ <= 0 && phase_ == 2)
        {
            stage2_bgm_pending_ = false;
            stop_active_sfx();
            play_stage2_bgm(true);
            rebirth_audio_timer_ = -1;
        }
    }

    if (player.damage_cooldown > 0)
        player.damage_cooldown--;
}

void Boss::draw() const
{
    bitmap img = img_regular_;
    switch (state_)
    {
    case State::PhaseHalfCue:
        img = (state_timer_ < 84) ? img_half0_ : img_half1_;
        break;
    case State::LowHpCue:
        img = img_lowhp_;
        break;
    case State::Phase1Death:
    case State::DeadFinal:
        img = img_dead_;
        break;
    case State::Rebirth:
        img = img_rebirth_;
        break;
    case State::PlayerLose:
        img = img_player_lose_;
        break;
    default:
        break;
    }

    if (img)
        draw_bitmap(img, x, y);
    else
        fill_rectangle(enraged() ? COLOR_RED : COLOR_GRAY, x, y, width, height);

    for (const auto &shot : shots_)
    {
        if (!shot.active) continue;
        double ang_deg = std::atan2(shot.dy, shot.dx) * 180.0 / PI;
        double local_x = shot.x - x;
        double local_y = shot.y - y;
        bool inside_sprite = (local_x >= 0 && local_x <= width && local_y >= 0 && local_y <= height);
        if (inside_sprite) continue;

        if (img_bullet_fan_)
            draw_bitmap(img_bullet_fan_, shot.x, shot.y, option_rotate_bmp(ang_deg));
        else
            fill_circle(COLOR_ORANGE, shot.x, shot.y, 4.0);
    }

    if (state_ == State::P2_Lasers || state_ == State::P2_LasersGrow)
    {
        double cx = x + width / 2.0;
        double cy = y + height / 2.0;
        double inner = lasers_inner_radius_;
        double len = (state_ == State::P2_LasersGrow) ? lasers_current_length_ : LASER_MAX_LENGTH;
        for (const auto &laser : lasers_)
        {
            double dirx = std::cos(laser.ang);
            double diry = std::sin(laser.ang);
            double sx = cx + dirx * inner;
            double sy = cy + diry * inner;
            double ex = cx + dirx * len;
            double ey = cy + diry * len;
            draw_line(COLOR_RED, sx, sy, ex, ey, option_line_width(4));
        }
    }
}
