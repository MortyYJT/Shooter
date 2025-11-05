#pragma once
#include <string>

struct MenuState {
    bool in_menu = true;
    bool has_save = false;
    int selected = 0; // 0..N-1
};

enum class MenuAction {
    None,
    NewGame,
    Continue,
    Quit
};

void init_menu(MenuState &m, bool has_save);
MenuAction update_menu(MenuState &m);
void draw_menu(const MenuState &m, int screen_w, int screen_h);

