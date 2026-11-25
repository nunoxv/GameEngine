#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

// Thin RAII facade over Box2D v3.
// All public coordinates use pixels; conversion to Box2D meters happens internally.
// Hides <box2d/box2d.h> behind Pimpl so game code never depends on Box2D headers.
namespace xe {

    enum class BodyType { Static, Kinematic, Dynamic };

    // Describes a collision shape attached to a PhysicsBody
    struct ShapeDesc {
        enum class Kind { Box, Circle };
        Kind kind = Kind::Box;
        glm::vec2 halfExtents{ 8.f, 8.f };   // Half-width and half-height in pixels (for Box)
        float radius = 8.f;                // Radius in pixels (for Circle)
        bool sensor = false;               // Sensors detect overlaps without physical bounce/push
        std::uint16_t category = 0x0001;   // Collision layer bitmask (what I am)
        std::uint16_t mask = 0xFFFF;       // Collision filter bitmask (what I can hit)
    };

    struct BodyDesc {
        BodyType type = BodyType::Kinematic;
        glm::vec2 position{ 0.f };           // Initial center position in pixels
    };

    // RAII wrapper owning a single Box2D body (b2BodyId). Automatically destroyed when Actor dies.
    class   PhysicsBody {
    public:
        struct Impl;
        explicit PhysicsBody(std::unique_ptr<Impl> impl);
        ~PhysicsBody();

        PhysicsBody(const PhysicsBody&) = delete;
        PhysicsBody& operator=(const PhysicsBody&) = delete;

        void addShape(const ShapeDesc& shape);
        glm::vec2 position() const;
        void setPosition(glm::vec2 p);
        void setVelocity(glm::vec2 v);

    private:
        std::unique_ptr<Impl> impl_;
    };

    // RAII wrapper owning a Box2D simulation world (b2WorldId) and its sensor overlap events.
    class PhysicsWorld {
    public:
        struct Overlap {
            void* sensor;    // User data pointer (Actor*) of the sensor shape
            void* visitor;   // User data pointer (Actor*) of the shape that entered the sensor
        };

        PhysicsWorld();
        ~PhysicsWorld();

        PhysicsWorld(const PhysicsWorld&) = delete;
        PhysicsWorld& operator=(const PhysicsWorld&) = delete;

        std::unique_ptr<PhysicsBody> createBody(const BodyDesc& desc, void* userData);
        void step(float dt);                           // Advances physics by one fixed timestep
        const std::vector<Overlap>& overlaps() const;  // Sensor overlaps recorded during the last step

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

}  // namespace xe