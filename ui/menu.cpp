#include "menu.hpp"
#include "splashkit.h"

void init_menu(MenuState &m, bool has_save) {
    m.in_menu = true;
    m.has_save = has_save;
    m.selected = 0;
}

MenuAction update_menu(MenuState &m) {
    int max_index = m.has_save ? 3 : 2; // 0..2 (New, Continue?, Quit)
    // input
    if (key_typed(DOWN_KEY) || key_typed(S_KEY)) {
        m.selected = (m.selected + 1) % max_index;
    }
    if (key_typed(UP_KEY) || key_typed(W_KEY)) {
        m.selected = (m.selected - 1 + max_index) % max_index;
    }
    if (key_typed(RETURN_KEY) || key_typed(SPACE_KEY)) {
        if (m.selected == 0) return MenuAction::NewGame;
        if (m.has_save) {
            if (m.selected == 1) return MenuAction::Continue;
            if (m.selected == 2) return MenuAction::Quit;
        } else {
            if (m.selected == 1) return MenuAction::Quit;
        }
    }
    return MenuAction::None;
}

void draw_menu(const MenuState &m, int screen_w, int screen_h) {
    // simple full-screen dim
    fill_rectangle(rgba_color(0,0,0,180), 0, 0, screen_w, screen_h);
    draw_text("Shooter", COLOR_WHITE, "arial", 64, screen_w/2 - 120, screen_h/2 - 220);

    int y = screen_h/2 - 60;
    int x = screen_w/2 - 160;
    int line_gap = 56;

    // Menu options
    color c0 = (m.selected == 0) ? COLOR_YELLOW : COLOR_WHITE;
    draw_text("New Game", c0, "arial", 36, x, y);
    int idx = 1;
    if (m.has_save) {
        color c1 = (m.selected == 1) ? COLOR_YELLOW : COLOR_WHITE;
        draw_text("Continue", c1, "arial", 36, x, y + line_gap * idx);
        idx++;
    }
    color cQ = (m.selected == (m.has_save ? 2 : 1)) ? COLOR_YELLOW : COLOR_WHITE;
    draw_text("Quit", cQ, "arial", 36, x, y + line_gap * idx);

    draw_text("Use W/S or Up/Down, Enter to select", COLOR_WHITE, "arial", 22, x, y + line_gap * (idx + 2));
}

