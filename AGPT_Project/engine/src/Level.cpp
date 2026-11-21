#include <xe/Level.h>

#include <algorithm>
#include <xe/Renderer.h>

namespace xe {

    Level::Level(Engine& engine) : engine_(engine) {}
    Level::~Level() = default;

    void Level::flushPending() {
        for (auto& p : pending_) {
            actors_.push_back(std::move(p));
        }
        pending_.clear();
    }

    Actor* Level::find(std::uint64_t id) {
        for (const auto& a : actors_) {
            if (a->id_ == id) return a->alive_ ? a.get() : nullptr;
        }
        for (const auto& a : pending_) {
            if (a->id_ == id) return a->alive_ ? a.get() : nullptr;
        }
        return nullptr;
    }

    void Level::update(float dt) {
        flushPending();
        onUpdate(dt);

        // Index-based loop is safe even if an actor's update() calls spawn(),
        // because newly spawned actors go into `pending_` rather than resizing `actors_`.
        for (std::size_t i = 0; i < actors_.size(); ++i) {
            Actor& a = *actors_[i];
            if (!a.alive_) continue;
            a.animator.update(dt);
            a.update(dt);
        }
    }

    void Level::fixedUpdate(float dt) {
        // 1. Advance Box2D physics by one fixed step
        physics_.step(dt);

        // 2. Copy updated physics positions back to each actor's visual transform
        for (auto& a : actors_) {
            if (a->alive_ && a->body_) {
                a->position = a->body_->position();
            }
        }

        // 3. Notify both actors involved in any Box2D sensor overlap
        for (const auto& o : physics_.overlaps()) {
            Actor* sensor = static_cast<Actor*>(o.sensor);
            Actor* visitor = static_cast<Actor*>(o.visitor);
            if (!sensor || !visitor || !sensor->alive_ || !visitor->alive_) continue;

            // Even if `sensor->onOverlap()` calls `sensor->destroy()`, `sensor` remains valid
            // in memory until cleanup(), so `visitor->onOverlap(*sensor)` can still inspect it safely.
            sensor->onOverlap(*visitor);
            if (visitor->alive_) {
                visitor->onOverlap(*sensor);
            }
        }

        // 4. Post-physics hook (e.g., clamping the player ship to screen bounds)
        for (auto& a : actors_) {
            if (a->alive_ && a->body_) {
                a->afterPhysics();
            }
        }
    }

    void Level::cleanup() {
        flushPending();

        // Erase-remove idiom: erases all dead actors in O(n) time.
        // Destroying a unique_ptr<Actor> automatically destroys its PhysicsBody via RAII.
        actors_.erase(
            std::remove_if(actors_.begin(), actors_.end(),
                [](const std::unique_ptr<Actor>& a) { return !a->alive_; }),
            actors_.end());
    }

    void Level::render(Renderer& renderer) {
        // Collect surviving actors into a temporary list so sorting by `layer`
        // does not alter the update order of `actors_`.
        drawList_.clear();
        for (auto& a : actors_) {
            if (a->alive_) {
                drawList_.push_back(a.get());
            }
        }

        std::stable_sort(drawList_.begin(), drawList_.end(),
            [](const Actor* a, const Actor* b) { return a->layer < b->layer; });

        for (Actor* a : drawList_) {
            a->render(renderer);
        }
    }

}  // namespace xe