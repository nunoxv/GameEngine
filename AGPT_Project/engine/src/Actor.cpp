#include <xe/Actor.h>

#include <xe/Level.h>
#include <xe/Renderer.h>

namespace xe {

    Actor::Actor(Level& level) : level_(level) {}

    // Defaulted destructor triggers ~unique_ptr<PhysicsBody>(), freeing the Box2D body via RAII.
    Actor::~Actor() = default;

    void Actor::createBody(BodyType type) {
        BodyDesc desc;
        desc.type = type;
        desc.position = position;

        // Pass `this` as user data so Box2D sensor events map back to this Actor instance
        body_ = level_.physics().createBody(desc, this);
    }

    void Actor::render(Renderer& renderer) {
        // Skip rendering if no sprite sheet is assigned (e.g., invisible trigger/spawner actors)
        if (!sheet) return;

        // Let the Animator select the active tile index whenever a clip is playing
        if (animator.hasClip()) {
            frame = animator.frame();
        }

        // Build a backend-agnostic draw command (zero OpenGL or SDL types exposed here)
        SpriteDraw d;
        d.texture = &sheet->texture();
        d.uv = sheet->frameUV(frame);
        d.position = position;
        d.size = (spriteSize.x > 0.f && spriteSize.y > 0.f) ? spriteSize : sheet->frameSize();
        d.rotation = rotation;
        d.scale = scale;
        d.tint = tint;

        renderer.drawSprite(d);
    }

}  // namespace xe