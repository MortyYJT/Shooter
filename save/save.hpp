#pragma once
#include <vector>
#include <string>

struct SaveData {
    int version = 1;
    int money = 0;
    int wave = 1;
    // weapon types for two slots: -1 if empty, else 0..5
    int slot_types[2] = {-1, -1};
    int current_slot = 0; // 0 or 1
    // shop unlock flags (first 4 used: Pistol, AK, Shotgun, AWP)
    bool unlocked[6] = {true, true, false, false, false, false};
};

// Path to save file (relative to project root)
std::string save_file_path();

bool save_exists();
bool save_game(const SaveData &data);
bool load_game(SaveData &out);
bool delete_save();
