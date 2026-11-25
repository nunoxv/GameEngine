#pragma once

#include <cstdint>
#include <memory>
#include <glm/glm.hpp>
#include <xe/Animation.h>
#include <xe/Physics.h>
#include <xe/SpriteSheet.h>

namespace xe {

    class Level;
    class Renderer;

    // Base class for all entities in a Level (spaceship, enemies, missiles, explosions).
    // Owned exclusively by its Level via std::unique_ptr; external code refers to actors
    // either by unique id() or short-lived raw pointers within the same frame.
    class Actor {
    public:
        explicit Actor(Level& level);
        virtual ~Actor();

        // Non-copyable: each Actor owns a unique Box2D body and has a unique Level ID.
        Actor(const Actor&) = delete;
        Actor& operator=(const Actor&) = delete;

        // Lifecycle hooks overridden by concrete game actors:
        virtual void update(float) {}             // Input & gameplay logic (once per frame)
        virtual void afterPhysics() {}            // Post-Box2D adjustments (e.g., screen clamping)
        virtual void onOverlap(Actor&) {}         // Triggered when Box2D sensor shapes overlap
        virtual void render(Renderer& renderer);  // Submits sprite draw data to the abstract Renderer

        // Deferred destruction: marks the actor dead so Level safely deletes it at the end
        // of the frame without invalidating iterators mid-update or mid-collision callback.
        void destroy() { alive_ = false; }
        bool alive() const { return alive_; }
        std::uint64_t id() const { return id_; }

        Level& level() { return level_; }
        const Level& level() const { return level_; }

        bool hasBody() const { return body_ != nullptr; }
        PhysicsBody& body() { return *body_; }
        const PhysicsBody& body() const { return *body_; }

        // Visual transform & appearance properties (in pixel space and radians)
        glm::vec2 position{ 0.f };
        float rotation = 0.f;
        glm::vec2 scale{ 1.f };
        glm::vec2 spriteSize{ 0.f };   // If (0, 0), defaults to the SpriteSheet's single-frame pixel size
        glm::vec4 tint{ 1.f };         // RGBA color multiplier (default white = unmodified texture color)
        int layer = 0;               // Render sorting order (lower numbers draw first / further back)
        int frame = 0;               // Manual sprite-sheet frame index when no animation clip is active

        // Shared texture atlas (cached by ResourceManager) and per-actor animation state machine
        std::shared_ptr<SpriteSheet> sheet;
        Animator animator;

    protected:
        // Helper for subclasses to attach a Box2D physics body at the actor's current `position`
        void createBody(BodyType type);

    private:
        friend class Level;  // Allows Level to assign `id_` upon actor creation
        Level& level_;
        std::unique_ptr<PhysicsBody> body_;  // RAII: destroys Box2D body automatically on Actor deletion
        bool alive_ = true;
        std::uint64_t id_ = 0;
    };

}  // namespace xe