#pragma once
#include <cstdint>

// ---------------------------------------------------------------------------------------
// Sprite-sheet layout. Open each BMP and, if a sheet does not look right in game, adjust
// the columns/rows here (engine warns on stderr if the texture is not divisible by them).
// ---------------------------------------------------------------------------------------
struct SheetInfo {
    const char* file;
    int cols;
    int rows;
};

inline constexpr SheetInfo kShipSheet{"Ship1.bmp", 6, 1};      // 0-2 turning left, 3-5 turning right
inline constexpr int kShipIdleFrame = 2;
inline constexpr SheetInfo kMissileSheet{"missile.bmp", 4, 4}; // row 0 = light missiles
inline constexpr int kMissileRow0Frames = 4;
inline constexpr SheetInfo kLonerSheet{"LonerA.bmp", 4, 4};
inline constexpr SheetInfo kRusherSheet{"rusher.bmp", 4, 4};
inline constexpr SheetInfo kEnemyShotSheet{"EnWeap6.bmp", 4, 1};
inline constexpr const char* kExplosionFile = "explode64.bmp";  // 64x64 frames
inline constexpr int kExplosionFrameSize = 64;

// Optional background art; procedural star fields are used when these files are missing.
inline constexpr const char* kBackground0File = "bg_layer0.bmp";
inline constexpr const char* kBackground1File = "bg_layer1.bmp";

// Collision categories (Box2D filter bits)
namespace cat {
inline constexpr std::uint16_t Ship = 0x01;
inline constexpr std::uint16_t ShipSensor = 0x02;
inline constexpr std::uint16_t Missile = 0x04;
inline constexpr std::uint16_t Enemy = 0x08;
inline constexpr std::uint16_t EnemyShot = 0x10;
inline constexpr std::uint16_t All = 0xFFFF;
}  // namespace cat

// Draw order
namespace zorder {
inline constexpr int Back0 = -20, Back1 = -10, Enemy = 0, Shot = 2, Ship = 5, Fx = 10;
}
