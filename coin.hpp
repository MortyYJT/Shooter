#pragma once
#include "splashkit.h"
#include <vector>

// Represents a coin dropped by enemies
struct Coin
{
    double x, y;               // Position of the coin
    double value;              // Amount of money this coin gives
    bool active = false;       // Whether the coin is active/visible

    // Animation data
    std::vector<bitmap> frames; // All animation frames for the coin
    int frame = 0;              // Current frame index
    int frame_timer = 0;        // Timer for frame switching
    int frame_interval = 0;     // Frames between animation updates
    int frame_count = 0;        // Total number of animation frames
};
