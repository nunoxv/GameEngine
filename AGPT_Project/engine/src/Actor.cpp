#include <xe/Actor.h>

#include <xe/Level.h>
#include <xe/Renderer.h>

namespace xe {

Actor::Actor(Level& level) : level_(level) {}
Actor::~Actor() = default;   // body_ destroys its Box2D body

void Actor::createBody(BodyType type) {
    BodyDesc desc;
    desc.type = type;
    desc.position = position;
    body_ = level_.physics().createBody(desc, static_cast<void*>(this));
}

void Actor::render(Renderer& renderer) {
    if (!sheet) return;
    if (animator.hasClip()) frame = animator.frame();
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
