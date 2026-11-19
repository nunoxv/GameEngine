#pragma once
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <xe/Actor.h>
#include <xe/Physics.h>

namespace xe {

class Engine;
class Renderer;

// A Level owns its actors and its physics world.
// Member order matters: actors are destroyed before the physics world.
class Level {
public:
    explicit Level(Engine& engine);
    virtual ~Level();
    Level(const Level&) = delete;
    Level& operator=(const Level&) = delete;

    // Creates an actor (constructed as T(Level&, args...)). It joins the level at the next
    // flush, so it is safe to call from inside update() or collision callbacks.
    template <class T, class... Args>
    T* spawn(Args&&... args) {
        static_assert(std::is_base_of<Actor, T>::value, "T must derive from xe::Actor");
        auto owned = std::make_unique<T>(*this, std::forward<Args>(args)...);
        T* raw = owned.get();
        raw->id_ = nextId_++;
        pending_.push_back(std::move(owned));
        return raw;
    }

    // Safe lookup by id (nullptr when the actor is gone or dead).
    Actor* find(std::uint64_t id);

    virtual void onLoad() {}               // called once when the level becomes active
    virtual void onUpdate(float dt) { (void)dt; }

    // Driven by the Engine game loop
    void update(float dt);        // every actor: animation + update()
    void fixedUpdate(float dt);   // Box2D step, body sync, overlap dispatch
    void cleanup();               // removes dead actors, adds pending ones
    void render(Renderer& renderer);

    Engine& engine() { return engine_; }
    PhysicsWorld& physics() { return physics_; }

private:
    void flushPending();

    Engine& engine_;
    PhysicsWorld physics_;
    std::vector<std::unique_ptr<Actor>> actors_;
    std::vector<std::unique_ptr<Actor>> pending_;
    std::vector<Actor*> drawList_;
    std::uint64_t nextId_ = 1;
};

}  // namespace xe
