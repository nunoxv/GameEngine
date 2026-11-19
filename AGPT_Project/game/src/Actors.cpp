#include "Actors.h"

#include <cmath>
#include <xe/Engine.h>
#include <xe/InputManager.h>
#include <xe/Level.h>
#include <xe/Renderer.h>
#include "GameAssets.h"
#include "XenonLevel.h"

namespace {

XenonLevel& game(xe::Actor& a) { return static_cast<XenonLevel&>(a.level()); }

glm::vec2 viewport(xe::Actor& a) { return a.level().engine().renderer().viewportSize(); }

xe::Animation loopAll(int frames, float frameTime) {
    xe::Animation a;
    for (int i = 0; i < frames; ++i) a.frames.push_back(i);
    a.frameTime = frameTime;
    a.loop = true;
    return a;
}

}  // namespace

// ---- Explosion -------------------------------------------------------------------------
Explosion::Explosion(xe::Level& level, glm::vec2 pos, float size) : Actor(level) {
    position = pos;
    scale = glm::vec2(size);
    layer = zorder::Fx;
    sheet = game(*this).assets().explosion;
    rotation = game(*this).randomFloat(0.f, 6.2831853f);   // random orientation per explosion
    xe::Animation boom = loopAll(sheet->frameCount(), 0.035f);
    boom.loop = false;   // one-shot
    animator.add("boom", boom);
    animator.play("boom");
}

void Explosion::update(float dt) {
    rotation += 1.5f * dt;   // visible rotation
    if (animator.finished()) destroy();
}

// ---- Enemies ---------------------------------------------------------------------------
void Enemy::takeHit(int damage) {
    health_ -= damage;
    if (health_ <= 0) {
        level().spawn<Explosion>(position, 1.2f);
        game(*this).addScore(points_);
        destroy();
    }
}

void Enemy::crash() {
    level().spawn<Explosion>(position, 1.2f);
    destroy();
}

Loner::Loner(xe::Level& level, float y, bool fromLeft) : Enemy(level), direction_(fromLeft ? 1.f : -1.f) {
    health_ = 3;
    points_ = 150;
    layer = zorder::Enemy;
    sheet = game(*this).assets().loner;
    const float half = sheet->frameSize().x * 0.5f;
    position = {fromLeft ? -half : viewport(*this).x + half, y};
    animator.add("fly", loopAll(sheet->frameCount(), 0.1f));
    animator.play("fly");

    createBody(xe::BodyType::Kinematic);
    xe::ShapeDesc s;
    s.halfExtents = sheet->frameSize() * 0.5f * 0.7f;
    s.category = cat::Enemy;
    s.mask = cat::All;
    body().addShape(s);
    body().setVelocity({direction_ * 130.f, 0.f});
}

void Loner::update(float dt) {
    const float half = sheet->frameSize().x * 0.5f;
    if (position.x < -half - 20.f || position.x > viewport(*this).x + half + 20.f) {
        destroy();
        return;
    }
    fireTimer_ -= dt;
    if (fireTimer_ <= 0.f) {
        fireTimer_ += 2.f;   // every 2 seconds
        glm::vec2 dir{0.f, 1.f};
        if (Ship* ship = game(*this).ship()) {
            const glm::vec2 d = ship->position - position;
            if (glm::length(d) > 1.f) dir = glm::normalize(d);
        }
        level().spawn<EnemyShot>(position, dir * 240.f);
    }
}

Rusher::Rusher(xe::Level& level, float x) : Enemy(level) {
    health_ = 2;
    points_ = 100;
    layer = zorder::Enemy;
    sheet = game(*this).assets().rusher;
    position = {x, -sheet->frameSize().y * 0.5f};
    animator.add("fly", loopAll(sheet->frameCount(), 0.1f));
    animator.play("fly");

    createBody(xe::BodyType::Kinematic);
    xe::ShapeDesc s;
    s.halfExtents = sheet->frameSize() * 0.5f * 0.7f;
    s.category = cat::Enemy;
    s.mask = cat::All;
    body().addShape(s);
    body().setVelocity({0.f, 230.f});
}

void Rusher::update(float) {
    if (position.y > viewport(*this).y + sheet->frameSize().y) destroy();
}

// ---- Projectiles -----------------------------------------------------------------------
EnemyShot::EnemyShot(xe::Level& level, glm::vec2 pos, glm::vec2 velocity) : Actor(level) {
    position = pos;
    layer = zorder::Shot;
    sheet = game(*this).assets().enemyShot;
    animator.add("fly", loopAll(sheet->frameCount(), 0.08f));
    animator.play("fly");

    createBody(xe::BodyType::Kinematic);
    xe::ShapeDesc s;
    s.kind = xe::ShapeDesc::Kind::Circle;
    s.radius = sheet->frameSize().x * 0.4f;
    s.sensor = true;
    s.category = cat::EnemyShot;
    s.mask = cat::Ship;
    body().addShape(s);
    body().setVelocity(velocity);
}

void EnemyShot::update(float dt) {
    rotation += 8.f * dt;   // spinning projectile
    const glm::vec2 vp = viewport(*this);
    if (position.x < -40.f || position.x > vp.x + 40.f || position.y < -40.f || position.y > vp.y + 40.f)
        destroy();
}

void EnemyShot::onOverlap(xe::Actor& other) {
    if (auto* s = dynamic_cast<Ship*>(&other)) {
        s->hit();
        level().spawn<Explosion>(position, 0.6f);
        destroy();
    }
}

Missile::Missile(xe::Level& level, glm::vec2 pos) : Actor(level) {
    position = pos;
    layer = zorder::Shot;
    sheet = game(*this).assets().missile;
    animator.add("fly", loopAll(kMissileRow0Frames, 0.06f));   // first row = light missiles
    animator.play("fly");

    createBody(xe::BodyType::Kinematic);
    xe::ShapeDesc s;
    s.halfExtents = sheet->frameSize() * 0.5f * 0.6f;
    s.sensor = true;
    s.category = cat::Missile;
    s.mask = cat::Enemy;
    body().addShape(s);
    body().setVelocity({0.f, -650.f});
}

void Missile::update(float) {
    if (position.y < -60.f) destroy();
}

void Missile::onOverlap(xe::Actor& other) {
    if (auto* e = dynamic_cast<Enemy*>(&other)) {
        e->takeHit(1);
        level().spawn<Explosion>(position, 0.5f);
        destroy();
    }
}

// ---- Ship ------------------------------------------------------------------------------
Ship::Ship(xe::Level& level, glm::vec2 pos) : Actor(level) {
    position = pos;
    layer = zorder::Ship;
    sheet = game(*this).assets().ship;

    xe::Animation idle;
    idle.frames = {kShipIdleFrame};
    idle.loop = false;
    animator.add("idle", idle);
    xe::Animation left;
    left.frames = {2, 1, 0};
    left.frameTime = 0.06f;
    left.loop = false;
    animator.add("left", left);
    xe::Animation right;
    right.frames = {3, 4, 5};
    right.frameTime = 0.06f;
    right.loop = false;
    animator.add("right", right);
    animator.play("idle");

    createBody(xe::BodyType::Kinematic);
    xe::ShapeDesc solid;   // visitor shape: enemy shots detect this one
    solid.halfExtents = sheet->frameSize() * 0.5f * 0.5f;
    solid.category = cat::Ship;
    solid.mask = cat::All;
    body().addShape(solid);
    xe::ShapeDesc sensor;  // detects enemies ramming the ship
    sensor.halfExtents = sheet->frameSize() * 0.5f * 0.6f;
    sensor.sensor = true;
    sensor.category = cat::ShipSensor;
    sensor.mask = cat::Enemy;
    body().addShape(sensor);
}

void Ship::update(float dt) {
    auto& input = level().engine().input();
    glm::vec2 dir{input.axis("move_x"), input.axis("move_y")};
    if (glm::length(dir) > 1.f) dir = glm::normalize(dir);
    body().setVelocity(dir * 320.f);

    if (dir.x < -0.2f) animator.play("left");
    else if (dir.x > 0.2f) animator.play("right");
    else animator.play("idle");

    fireCooldown_ -= dt;
    if (input.isDown("fire") && fireCooldown_ <= 0.f) {
        fireCooldown_ = 0.18f;
        level().spawn<Missile>(position + glm::vec2(0.f, -sheet->frameSize().y * 0.5f));
    }

    if (invulnerable_ > 0.f) {
        invulnerable_ -= dt;
        tint.a = (static_cast<int>(invulnerable_ * 10.f) % 2) ? 0.35f : 1.f;   // blink
    } else {
        tint.a = 1.f;
    }
}

void Ship::afterPhysics() {
    const glm::vec2 half = sheet->frameSize() * 0.5f;
    const glm::vec2 vp = viewport(*this);
    const glm::vec2 clamped{std::fmin(std::fmax(position.x, half.x), vp.x - half.x),
                            std::fmin(std::fmax(position.y, half.y), vp.y - half.y)};
    if (clamped != position) {
        position = clamped;
        body().setPosition(clamped);
    }
}

void Ship::onOverlap(xe::Actor& other) {
    if (auto* e = dynamic_cast<Enemy*>(&other)) {
        if (invulnerable_ <= 0.f) hit();
        e->crash();
    }
}

void Ship::hit() {
    if (invulnerable_ > 0.f || !alive()) return;
    invulnerable_ = 1.5f;
    level().spawn<Explosion>(position, 0.6f);
    if (--health_ <= 0) {
        level().spawn<Explosion>(position, 1.6f);
        game(*this).onShipDestroyed();
        destroy();
    }
}
