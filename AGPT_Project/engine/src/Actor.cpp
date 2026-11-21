#include <xe/Actor.h>
#include <xe/Level.h>
#include <xe/Renderer.h>

namespace xe {

    Actor::Actor(Level& level) : level_(level) {}

    // Defaulted destructor automatically triggers ~unique_ptr<PhysicsBody>(),
    // guaranteeing the underlying Box2D body is cleaned up via RAII.
    Actor::~Actor() = default;

    void Actor::createBody(BodyType type) {
        BodyDesc desc;
        desc.type = type;
        desc.position = position;

        // Pass `this` as user data so Box2D collision events can map back to this Actor instance
        body_ = level_.physics().createBody(desc, static_cast<void*>(this));
    }

    void Actor::render(Renderer& renderer) {
        // Skip rendering if no sprite sheet has been assigned (e.g., invisible trigger/spawner actors)
        if (!sheet) return;

        // If an animation clip is currently playing, let the Animator dictate the active frame
        if (animator.hasClip()) {
            frame = animator.frame();
        }

        // Build a backend-agnostic draw command (no OpenGL or SDL types exposed here)
        SpriteDraw d;
        d.texture = &sheet->texture();
        d.uv = sheet->frameUV(frame); // Computes normalized UV rectangle for the chosen tile
        d.position = position;
        d.size = (spriteSize.x > 0.f && spriteSize.y > 0.f) ? spriteSize : sheet->frameSize();
        d.rotation = rotation;
        d.scale = scale;
        d.tint = tint;

        renderer.drawSprite(d);
    }

}  // namespace xe