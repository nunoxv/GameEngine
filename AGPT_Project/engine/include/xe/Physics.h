#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

// Thin RAII facade over Box2D. Units are pixels; the conversion to metres is internal.
namespace xe {

enum class BodyType { Static, Kinematic, Dynamic };

struct ShapeDesc {
    enum class Kind { Box, Circle };
    Kind kind = Kind::Box;
    glm::vec2 halfExtents{8.f, 8.f};   // Box
    float radius = 8.f;                // Circle
    bool sensor = false;               // sensors report overlaps, never collide physically
    std::uint16_t category = 0x0001;
    std::uint16_t mask = 0xFFFF;
};

struct BodyDesc {
    BodyType type = BodyType::Kinematic;
    glm::vec2 position{0.f};
};

class PhysicsBody {
public:
    ~PhysicsBody();   // destroys the b2Body
    PhysicsBody(const PhysicsBody&) = delete;
    PhysicsBody& operator=(const PhysicsBody&) = delete;

    void addShape(const ShapeDesc& shape);
    glm::vec2 position() const;
    void setPosition(glm::vec2 p);
    glm::vec2 velocity() const;
    void setVelocity(glm::vec2 v);

private:
    friend class PhysicsWorld;
    struct Impl;
    explicit PhysicsBody(std::unique_ptr<Impl> impl);
    std::unique_ptr<Impl> impl_;
};

class PhysicsWorld {
public:
    struct Overlap {
        void* sensor;    // user data of the sensor shape's owner
        void* visitor;   // user data of the shape that entered it
    };

    PhysicsWorld();    // b2CreateWorld
    ~PhysicsWorld();   // b2DestroyWorld
    PhysicsWorld(const PhysicsWorld&) = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;

    std::unique_ptr<PhysicsBody> createBody(const BodyDesc& desc, void* userData);
    void step(float dt);                           // fixed time step
    const std::vector<Overlap>& overlaps() const;  // sensor "begin overlap" events of the last step

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace xe
