#include <xe/Level.h>

#include <algorithm>
#include <xe/Renderer.h>

namespace xe {

Level::Level(Engine& engine) : engine_(engine) {}
Level::~Level() = default;

void Level::flushPending() {
    for (auto& p : pending_) actors_.push_back(std::move(p));
    pending_.clear();
}

Actor* Level::find(std::uint64_t id) {
    for (auto& a : actors_)
        if (a->id_ == id) return a->alive_ ? a.get() : nullptr;
    for (auto& a : pending_)
        if (a->id_ == id) return a->alive_ ? a.get() : nullptr;
    return nullptr;
}

void Level::update(float dt) {
    flushPending();
    onUpdate(dt);
    // Index loop: update() may spawn, but spawns go to pending_, never into actors_.
    for (std::size_t i = 0; i < actors_.size(); ++i) {
        Actor& a = *actors_[i];
        if (!a.alive_) continue;
        a.animator.update(dt);
        a.update(dt);
    }
}

void Level::fixedUpdate(float dt) {
    physics_.step(dt);
    for (auto& a : actors_)
        if (a->alive_ && a->body_) a->position = a->body_->position();

    for (const auto& o : physics_.overlaps()) {
        Actor* sensor = static_cast<Actor*>(o.sensor);
        Actor* visitor = static_cast<Actor*>(o.visitor);
        if (!sensor || !visitor || !sensor->alive_ || !visitor->alive_) continue;
        sensor->onOverlap(*visitor);
        if (visitor->alive_) visitor->onOverlap(*sensor);
    }

    for (auto& a : actors_)
        if (a->alive_ && a->body_) a->afterPhysics();
}

void Level::cleanup() {
    flushPending();
    // Destroying an actor destroys its Box2D body (RAII). This never happens inside a step.
    actors_.erase(std::remove_if(actors_.begin(), actors_.end(),
                                 [](const std::unique_ptr<Actor>& a) { return !a->alive_; }),
                  actors_.end());
}

void Level::render(Renderer& renderer) {
    drawList_.clear();
    for (auto& a : actors_)
        if (a->alive_) drawList_.push_back(a.get());
    std::stable_sort(drawList_.begin(), drawList_.end(),
                     [](const Actor* a, const Actor* b) { return a->layer < b->layer; });
    for (Actor* a : drawList_) a->render(renderer);
}

}  // namespace xe
