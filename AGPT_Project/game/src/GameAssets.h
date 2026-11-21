#pragma once

#include <cstdint>

// Describes a uniform sprite-sheet grid (columns x rows) inside a single BMP texture.
struct SheetInfo {
    const char* file;
    int cols;
    int rows;
};

// Sprite Sheets 
inline constexpr SheetInfo kShipSheet{ "Ship1.bmp", 7, 1 };       // Tiles 0..2: turn left, 3..5: turn right
inline constexpr int kShipIdleFrame = 3;                        // Neutral center frame when not banking
inline constexpr SheetInfo kMissileSheet{ "missile.bmp", 4, 4 };  // Row 0 (frames 0..3) = light missiles
inline constexpr int kMissileRow0Frames = 4;
inline constexpr SheetInfo kLonerSheet{ "LonerA.bmp", 4, 4 };
inline constexpr SheetInfo kRusherSheet{ "rusher.bmp", 4, 4 };
inline constexpr SheetInfo kEnemyShotSheet{ "EnWeap6.bmp", 4, 1 };
inline constexpr const char* kExplosionFile = "explode64.bmp";  // Sliced via SpriteSheet::fromFrameSize
inline constexpr int kExplosionFrameSize = 64;

// Optional scrolling background BMPs (procedural starfields are generated if not found)
inline constexpr const char* kBackground0File = "galaxy2.bmp";
inline constexpr const char* kBackground1File = "GAStarfield.bmp";

// --- Box2D Collision Category Bitmasks -------------------------------------------------
// Used with ShapeDesc::category and ShapeDesc::mask so Box2D filters pairs in C:
// e.g., Player Missiles only collide with Enemies, and EnemyShots only collide with Ship.
namespace cat {
    inline constexpr std::uint16_t Ship = 0x0001;
    inline constexpr std::uint16_t ShipSensor = 0x0002;
    inline constexpr std::uint16_t Missile = 0x0004;
    inline constexpr std::uint16_t Enemy = 0x0008;
    inline constexpr std::uint16_t EnemyShot = 0x0010;
    inline constexpr std::uint16_t All = 0xFFFF;
}  // namespace cat

// --- Render Sorting Layers (lower values draw first / further in the background) -------
namespace zorder {
    inline constexpr int Back0 = -20;  // Distant slow-scrolling parallax layer
    inline constexpr int Back1 = -10;  // Closer fast-scrolling parallax layer
    inline constexpr int Enemy = 0;  // Loners and Rushers
    inline constexpr int Shot = 2;  // Player missiles and enemy projectiles
    inline constexpr int Ship = 5;  // Player spaceship
    inline constexpr int Fx = 10;  // Explosions drawn on top of everything
}  // namespace zorder