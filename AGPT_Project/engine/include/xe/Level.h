#pragma once

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>
#include <xe/Actor.h>
#include <xe/Physics.h>

namespace xe {

    class Engine;
    class Renderer;

    // Owns all active Actors and the Box2D PhysicsWorld for a game scene.
    // Member declaration order is critical for RAII: `actors_` and `pending_` are declared
    // AFTER `physics_`, guaranteeing all Box2D bodies are destroyed BEFORE the Box2D world.
    class Level {
    public:
        explicit Level(Engine& engine);
        virtual ~Level();

        Level(const Level&) = delete;
        Level& operator=(const Level&) = delete;

        // Creates a new Actor of type T and queues it in `pending_`.
        // Queuing avoids modifying `actors_` while iterating over it during update() or collisions.
        template <class T, class... Args>
        T* spawn(Args&&... args) {
            auto owned = std::make_unique<T>(*this, std::forward<Args>(args)...);
            T* raw = owned.get();
            raw->id_ = nextId_++;
            pending_.push_back(std::move(owned));
            return raw;
        }

        // Safe lookup by unique ID (returns nullptr if the actor was destroyed or doesn't exist)
        Actor* find(std::uint64_t id);

        // Hooks overridden by concrete game levels (e.g., XenonLevel)
        virtual void onLoad() {}
        virtual void onUpdate(float dt) { (void)dt; }

        // Frame lifecycle methods driven by Engine::run()
        void update(float dt);        // Updates animations and actor logic
        void fixedUpdate(float dt);   // Steps Box2D, syncs transforms, dispatches overlaps
        void cleanup();               // Deletes dead actors and promotes newly spawned ones
        void render(Renderer& renderer);

        Engine& engine() { return engine_; }
        PhysicsWorld& physics() { return physics_; }

    private:
        void flushPending(); // Moves queued actors from `pending_` into active `actors_`

        Engine& engine_;
        PhysicsWorld physics_;                        // Destroyed LAST (after all actors)
        std::vector<std::unique_ptr<Actor>> actors_;  // Exclusive RAII ownership of live actors
        std::vector<std::unique_ptr<Actor>> pending_; // Staging buffer for mid-frame spawns
        std::vector<Actor*> drawList_;                // Reusable buffer for layer-sorted rendering
        std::uint64_t nextId_ = 1;
    };

}  // namespace xe