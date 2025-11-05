#include "save.hpp"
#include <fstream>
#include <filesystem>

static const char* kSaveRelPath = "save/save.dat";

std::string save_file_path() {
    return std::string(kSaveRelPath);
}

bool save_exists() {
    return std::filesystem::exists(save_file_path());
}

bool save_game(const SaveData &data) {
    std::ofstream ofs(save_file_path(), std::ios::trunc | std::ios::out);
    if (!ofs.is_open()) return false;
    ofs << "version " << data.version << "\n";
    ofs << "money " << data.money << "\n";
    ofs << "wave " << data.wave << "\n";
    ofs << "slot0 " << data.slot_types[0] << "\n";
    ofs << "slot1 " << data.slot_types[1] << "\n";
    ofs << "current " << data.current_slot << "\n";
    ofs << "unlocked ";
    for (int i = 0; i < 6; ++i) {
        ofs << (data.unlocked[i] ? 1 : 0);
        if (i < 5) ofs << ' ';
    }
    ofs << "\n";
    return true;
}

static bool read_token(std::istream &is, std::string &key) {
    if (!(is >> key)) return false;
    return true;
}

bool load_game(SaveData &out) {
    std::ifstream ifs(save_file_path());
    if (!ifs.is_open()) return false;
    std::string key;
    while (read_token(ifs, key)) {
        if (key == "version") ifs >> out.version;
        else if (key == "money") ifs >> out.money;
        else if (key == "wave") ifs >> out.wave;
        else if (key == "slot0") ifs >> out.slot_types[0];
        else if (key == "slot1") ifs >> out.slot_types[1];
        else if (key == "current") ifs >> out.current_slot;
        else if (key == "unlocked") {
            for (int i = 0; i < 6; ++i) {
                int v = 0; ifs >> v; out.unlocked[i] = (v != 0);
            }
        } else {
            // skip unknowns
            std::string dummy; std::getline(ifs, dummy);
        }
    }
    return true;
}

bool delete_save() {
    if (!save_exists()) return true;
    std::error_code ec;
    std::filesystem::remove(save_file_path(), ec);
    return !ec;
}

