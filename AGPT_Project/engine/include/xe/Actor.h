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

// Base class of everything that lives in a Level. Owned exclusively by its Level
// (std::unique_ptr); other code refers to actors by id() or by short-lived raw pointers.
class Actor {
public:
    explicit Actor(Level& level);
    virtual ~Actor();
    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;

    virtual void update(float dt) { (void)dt; }          // input + logic (once per frame)
    virtual void afterPhysics() {}                       // after each fixed physics step
    virtual void onOverlap(Actor& other) { (void)other; }// Box2D sensor begin-overlap
    virtual void render(Renderer& renderer);             // default: draws sheet/frame

    void destroy() { alive_ = false; }                   // removed at the end of the frame
    bool alive() const { return alive_; }
    std::uint64_t id() const { return id_; }
    Level& level() { return level_; }

    bool hasBody() const { return body_ != nullptr; }
    PhysicsBody& body() { return *body_; }

    // Transform / appearance (pixels, radians)
    glm::vec2 position{0.f};
    float rotation = 0.f;
    glm::vec2 scale{1.f};
    glm::vec2 spriteSize{0.f};   // (0,0) => use the sheet's frame size
    glm::vec4 tint{1.f};
    int layer = 0;               // draw order, lower first
    int frame = 0;               // used when no animation clip is active

    std::shared_ptr<SpriteSheet> sheet;
    Animator animator;

protected:
    void createBody(BodyType type);   // at the current `position`

private:
    friend class Level;
    Level& level_;
    std::unique_ptr<PhysicsBody> body_;
    bool alive_ = true;
    std::uint64_t id_ = 0;
};

}  // namespace xe
