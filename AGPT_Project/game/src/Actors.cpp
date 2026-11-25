#include "Actors.h"

#include <cstdint>
#include <xe/Engine.h>
#include <xe/InputManager.h>
#include <xe/Level.h>
#include <xe/Renderer.h>
#include "GameAssets.h"
#include "XenonLevel.h"

namespace {

    XenonLevel& game(xe::Actor& a) { return static_cast<XenonLevel&>(a.level()); }
    glm::vec2 viewport(xe::Actor& a) { return a.level().engine().renderer().viewportSize(); }

    // Builds an animation clip playing frames [0 .. count - 1] sequentially
    xe::Animation makeClip(int count, float frameTime, bool loop = true) {
        xe::Animation anim;
        for (int i = 0; i < count; ++i) anim.frames.push_back(i);
        anim.frameTime = frameTime;
        anim.loop = loop;
        return anim;
    }

    // Attaches an axis-aligned box collider or sensor to an actor's physics body
    void addBoxShape(xe::Actor& actor, glm::vec2 halfExtents, bool sensor,
        std::uint16_t category, std::uint16_t mask) {
        xe::ShapeDesc s;
        s.halfExtents = halfExtents;
        s.sensor = sensor;
        s.category = category;
        s.mask = mask;
        actor.body().addShape(s);
    }

    // Checks whether a position has left the visible screen plus a safety margin
    bool isOffScreen(glm::vec2 pos, glm::vec2 vp, float margin) {
        return pos.x < -margin || pos.x > vp.x + margin ||
            pos.y < -margin || pos.y > vp.y + margin;
    }

}  // namespace

// Explosion
Explosion::Explosion(xe::Level& level, glm::vec2 pos, float size) : xe::Actor(level) {
    position = pos;
    scale = glm::vec2(size);
    layer = zorder::Fx;
    sheet = game(*this).assets().explosion;
    rotation = game(*this).randomFloat(0.f, 6.2831853f);

    animator.add("boom", makeClip(sheet->frameCount(), 0.035f, false));
    animator.play("boom");
}

void Explosion::update(float dt) {
    rotation += 1.5f * dt;
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

Loner::Loner(xe::Level& level, float y, bool fromLeft)
    : Enemy(level), direction_(fromLeft ? 1.f : -1.f) {
    health_ = 3;
    points_ = 150;
    layer = zorder::Enemy;
    sheet = game(*this).assets().loner;

    const float halfW = sheet->frameSize().x * 0.5f;
    position = { fromLeft ? -halfW : viewport(*this).x + halfW, y };

    animator.add("fly", makeClip(sheet->frameCount(), 0.1f));
    animator.play("fly");

    // Non-sensor kinematic body so Missile and Ship sensors detect overlaps in Box2D v3
    createBody(xe::BodyType::Kinematic);
    addBoxShape(*this, sheet->frameSize() * 0.35f, false, cat::Enemy, cat::All);
    body().setVelocity({ direction_ * 130.f, 0.f });
}

void Loner::update(float dt) {
    if (isOffScreen(position, viewport(*this), sheet->frameSize().x * 0.5f + 20.f)) {
        destroy();
        return;
    }

    // Fire an aimed projectile at the player's current position every 2 seconds
    fireTimer_ -= dt;
    if (fireTimer_ <= 0.f) {
        fireTimer_ += 2.f;
        glm::vec2 dir{ 0.f, 1.f };
        if (Ship* ship = game(*this).ship()) {
            const glm::vec2 toShip = ship->position - position;
            if (glm::length(toShip) > 1.f) dir = glm::normalize(toShip);
        }
        level().spawn<EnemyShot>(position, dir * 240.f);
    }
}

Rusher::Rusher(xe::Level& level, float x) : Enemy(level) {
    health_ = 2;
    points_ = 100;
    layer = zorder::Enemy;
    sheet = game(*this).assets().rusher;
    position = { x, -sheet->frameSize().y * 0.5f };

    animator.add("fly", makeClip(sheet->frameCount(), 0.1f));
    animator.play("fly");

    createBody(xe::BodyType::Kinematic);
    addBoxShape(*this, sheet->frameSize() * 0.35f, false, cat::Enemy, cat::All);
    body().setVelocity({ 0.f, 230.f });
}

void Rusher::update(float) {
    if (isOffScreen(position, viewport(*this), sheet->frameSize().y)) destroy();
}

// ---- Projectiles -----------------------------------------------------------------------
EnemyShot::EnemyShot(xe::Level& level, glm::vec2 pos, glm::vec2 velocity) : xe::Actor(level) {
    position = pos;
    layer = zorder::Shot;
    sheet = game(*this).assets().enemyShot;

    animator.add("fly", makeClip(sheet->frameCount(), 0.08f));
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
    rotation += 8.f * dt;  // Continuous rotation
    if (isOffScreen(position, viewport(*this), 40.f)) destroy();
}

void EnemyShot::onOverlap(xe::Actor& other) {
    if (auto* ship = dynamic_cast<Ship*>(&other)) {
        ship->hit();
        level().spawn<Explosion>(position, 0.6f);
        destroy();
    }
}

Missile::Missile(xe::Level& level, glm::vec2 pos) : xe::Actor(level) {
    position = pos;
    layer = zorder::Shot;
    sheet = game(*this).assets().missile;

    // First row of missile.bmp = light missiles
    animator.add("fly", makeClip(kMissileRow0Frames, 0.06f));
    animator.play("fly");

    createBody(xe::BodyType::Kinematic);
    addBoxShape(*this, sheet->frameSize() * 0.3f, true, cat::Missile, cat::Enemy);
    body().setVelocity({ 0.f, -650.f });
}

void Missile::update(float) {
    if (isOffScreen(position, viewport(*this), 60.f)) destroy();
}

void Missile::onOverlap(xe::Actor& other) {
    if (auto* enemy = dynamic_cast<Enemy*>(&other)) {
        enemy->takeHit(1);
        level().spawn<Explosion>(position, 0.5f);
        destroy();
    }
}

// ---- Ship ------------------------------------------------------------------------------
Ship::Ship(xe::Level& level, glm::vec2 pos) : xe::Actor(level) {
    position = pos;
    layer = zorder::Ship;
    sheet = game(*this).assets().ship;

    // Ship1.bmp (7 tiles): 0..2 = turn left, 3 = neutral center, 4..6 = turn right
    animator.add("idle", { {kShipIdleFrame}, 0.1f, false });
    animator.add("left", { {2, 1, 0}, 0.06f, false });
    animator.add("right", { {4, 5, 6}, 0.06f, false });
    animator.play("idle");

    createBody(xe::BodyType::Kinematic);
    // Solid hitbox: allows EnemyShot sensors to trigger Box2D v3 sensor events
    addBoxShape(*this, sheet->frameSize() * 0.25f, false, cat::Ship, cat::All);
    // Sensor hitbox: detects ramming collisions against solid Enemy bodies
    addBoxShape(*this, sheet->frameSize() * 0.30f, true, cat::ShipSensor, cat::Enemy);
}

void Ship::update(float dt) {
    auto& input = level().engine().input();

    // Normalized movement vector from keyboard or gamepad stick
    glm::vec2 dir{ input.axis("move_x"), input.axis("move_y") };
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

    // Blink sprite alpha while invulnerable after taking damage
    if (invulnerable_ > 0.f) invulnerable_ -= dt;
    tint.a = (invulnerable_ > 0.f && (static_cast<int>(invulnerable_ * 10.f) % 2)) ? 0.35f : 1.f;
}

void Ship::afterPhysics() {
    const glm::vec2 half = sheet->frameSize() * 0.5f;
    const glm::vec2 clamped = glm::clamp(position, half, viewport(*this) - half);
    if (clamped != position) {
        position = clamped;
        body().setPosition(clamped);
    }
}

void Ship::onOverlap(xe::Actor& other) {
    if (auto* enemy = dynamic_cast<Enemy*>(&other)) {
        if (invulnerable_ <= 0.f) hit();
        enemy->crash();
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