// In-game pause menu: Continue, Save, Main Menu, Quit
#pragma once
#include "splashkit.h"
#include "../save/save.hpp"

enum class PauseAction { None, Continue, Save, MainMenu, Quit };

struct PauseState {
    bool active = false;
    int selected = 0; // 0..3
    bool saved_highlight = false; // after Save, paint Save option green until leaving pause
};

inline void init_pause(PauseState &p){ p.active = true; p.selected = 0; p.saved_highlight = false; }

inline PauseAction update_pause(PauseState &p)
{
    if (!p.active) return PauseAction::None;
    if (key_typed(DOWN_KEY) || key_typed(S_KEY)) p.selected = (p.selected + 1) % 4;
    if (key_typed(UP_KEY) || key_typed(W_KEY)) p.selected = (p.selected + 3) % 4;
    if (key_typed(RETURN_KEY) || key_typed(SPACE_KEY))
    {
        switch (p.selected)
        {
        case 0: return PauseAction::Continue;
        case 1: return PauseAction::Save;
        case 2: return PauseAction::MainMenu;
        case 3: return PauseAction::Quit;
        }
    }
    return PauseAction::None;
}

inline void draw_pause_menu(const PauseState &p, int screen_w, int screen_h)
{
    fill_rectangle(rgba_color(0,0,0,180), 0, 0, screen_w, screen_h);
    draw_text("Paused", COLOR_WHITE, "arial", 48, screen_w/2 - 90, screen_h/2 - 180);
    const char* items[4] = { "Continue", "Save", "Main Menu", "Quit" };
    int y = screen_h/2 - 60; int x = screen_w/2 - 140; int gap = 48;
    for (int i=0;i<4;++i)
    {
        color c = (p.selected == i) ? COLOR_YELLOW : COLOR_WHITE;
        if (i == 1 && p.saved_highlight) c = COLOR_GREEN;
        draw_text(items[i], c, "arial", 32, x, y + i*gap);
    }
    draw_text("Use W/S or Up/Down, Enter to select", COLOR_WHITE, "arial", 22, x, y + gap*3 + 64);
}
