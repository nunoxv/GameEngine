#pragma once
#include <glm/glm.hpp>
#include <xe/Actor.h>

class Explosion : public xe::Actor {
public:
    Explosion(xe::Level& level, glm::vec2 pos, float size);
    void update(float dt) override;
};

class Enemy : public xe::Actor {
public:
    using xe::Actor::Actor;
    virtual void takeHit(int damage);   // missile impact
    void crash();                       // rammed the ship: dies without score
protected:
    int health_ = 1;
    int points_ = 100;
};

// Moves horizontally and fires at the ship every 2 seconds.
class Loner : public Enemy {
public:
    Loner(xe::Level& level, float y, bool fromLeft);
    void update(float dt) override;
private:
    float fireTimer_ = 2.f;
    float direction_;
};

// Moves vertically (downwards).
class Rusher : public Enemy {
public:
    Rusher(xe::Level& level, float x);
    void update(float dt) override;
};

class EnemyShot : public xe::Actor {
public:
    EnemyShot(xe::Level& level, glm::vec2 pos, glm::vec2 velocity);
    void update(float dt) override;
    void onOverlap(xe::Actor& other) override;
};

class Missile : public xe::Actor {
public:
    Missile(xe::Level& level, glm::vec2 pos);
    void update(float dt) override;
    void onOverlap(xe::Actor& other) override;
};

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
