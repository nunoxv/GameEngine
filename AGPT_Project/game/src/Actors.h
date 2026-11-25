#pragma once

#include <glm/glm.hpp>
#include <xe/Actor.h>

// One-shot explosion effect spawned on missile hits, enemy crashes, and ship destruction.
class Explosion : public xe::Actor {
public:
    Explosion(xe::Level& level, glm::vec2 pos, float size);
    void update(float dt) override;
};

// Base class for enemies (Loner and Rusher) sharing health, score, and destruction logic.
class Enemy : public xe::Actor {
public:
    using xe::Actor::Actor;
    void takeHit(int damage);   // Damaged by player missile: awards score on death
    void crash();               // Collided directly with player ship: dies without score

protected:
    int health_ = 1;
    int points_ = 100;
};

// Moves horizontally across the screen and fires aimed projectiles every 2 seconds.
class Loner : public Enemy {
public:
    Loner(xe::Level& level, float y, bool fromLeft);
    void update(float dt) override;

private:
    float fireTimer_ = 2.f;
    float direction_;
};

// Moves vertically downward across the screen.
class Rusher : public Enemy {
public:
    Rusher(xe::Level& level, float x);
    void update(float dt) override;
};

// Projectile fired by Loners; visibly rotates and explodes on impact with the player ship.
class EnemyShot : public xe::Actor {
public:
    EnemyShot(xe::Level& level, glm::vec2 pos, glm::vec2 velocity);
    void update(float dt) override;
    void onOverlap(xe::Actor& other) override;
};

// Player-fired missile using the first row of missile.bmp.
class Missile : public xe::Actor {
public:
    Missile(xe::Level& level, glm::vec2 pos);
    void update(float dt) override;
    void onOverlap(xe::Actor& other) override;
};

// Player-controlled spaceship with keyboard/gamepad movement, turning animations, and firing.
class Ship : public xe::Actor {
public:
    Ship(xe::Level& level, glm::vec2 pos);
    void update(float dt) override;
    void afterPhysics() override;
    void onOverlap(xe::Actor& other) override;
    void hit();

private:
    float fireCooldown_ = 0.f;
    float invulnerable_ = 0.f;
    int health_ = 3;
};