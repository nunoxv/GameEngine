#include <xe/Physics.h>

#include <box2d/box2d.h>

namespace xe {
namespace {
constexpr float kPixelsPerMeter = 32.f;
b2Vec2 toB2(glm::vec2 v) { return {v.x / kPixelsPerMeter, v.y / kPixelsPerMeter}; }
glm::vec2 fromB2(b2Vec2 v) { return {v.x * kPixelsPerMeter, v.y * kPixelsPerMeter}; }
}  // namespace

// ---- PhysicsBody -----------------------------------------------------------------------
struct PhysicsBody::Impl {
    b2BodyId id = b2_nullBodyId;
    void* userData = nullptr;
};

PhysicsBody::PhysicsBody(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}

PhysicsBody::~PhysicsBody() {
    if (impl_ && b2Body_IsValid(impl_->id)) b2DestroyBody(impl_->id);   // RAII
}

void PhysicsBody::addShape(const ShapeDesc& d) {
    b2ShapeDef sd = b2DefaultShapeDef();
    sd.isSensor = d.sensor;
    sd.enableSensorEvents = true;
    sd.density = 1.f;
    sd.filter.categoryBits = d.category;
    sd.filter.maskBits = d.mask;
    sd.userData = impl_->userData;
    if (d.kind == ShapeDesc::Kind::Box) {
        b2Polygon box = b2MakeBox(d.halfExtents.x / kPixelsPerMeter, d.halfExtents.y / kPixelsPerMeter);
        b2CreatePolygonShape(impl_->id, &sd, &box);
    } else {
        b2Circle circle;
        circle.center = {0.f, 0.f};
        circle.radius = d.radius / kPixelsPerMeter;
        b2CreateCircleShape(impl_->id, &sd, &circle);
    }
}

glm::vec2 PhysicsBody::position() const { return fromB2(b2Body_GetPosition(impl_->id)); }
void PhysicsBody::setPosition(glm::vec2 p) {
    b2Body_SetTransform(impl_->id, toB2(p), b2Body_GetRotation(impl_->id));
}
glm::vec2 PhysicsBody::velocity() const { return fromB2(b2Body_GetLinearVelocity(impl_->id)); }
void PhysicsBody::setVelocity(glm::vec2 v) { b2Body_SetLinearVelocity(impl_->id, toB2(v)); }

// ---- PhysicsWorld ----------------------------------------------------------------------
struct PhysicsWorld::Impl {
    b2WorldId id = b2_nullWorldId;
    std::vector<Overlap> overlaps;
};

PhysicsWorld::PhysicsWorld() : impl_(std::make_unique<Impl>()) {
    b2WorldDef def = b2DefaultWorldDef();
    def.gravity = {0.f, 0.f};   // top-down shooter: no gravity
    impl_->id = b2CreateWorld(&def);
}

PhysicsWorld::~PhysicsWorld() {
    if (impl_ && b2World_IsValid(impl_->id)) b2DestroyWorld(impl_->id);
}

std::unique_ptr<PhysicsBody> PhysicsWorld::createBody(const BodyDesc& desc, void* userData) {
    b2BodyDef bd = b2DefaultBodyDef();
    switch (desc.type) {
        case BodyType::Static: bd.type = b2_staticBody; break;
        case BodyType::Kinematic: bd.type = b2_kinematicBody; break;
        case BodyType::Dynamic: bd.type = b2_dynamicBody; break;
    }
    bd.position = toB2(desc.position);
    bd.userData = userData;
    bd.enableSleep = false;   // sensors on sleeping bodies would not report overlaps
    auto impl = std::make_unique<PhysicsBody::Impl>();
    impl->id = b2CreateBody(impl_->id, &bd);
    impl->userData = userData;
    return std::unique_ptr<PhysicsBody>(new PhysicsBody(std::move(impl)));
}

void PhysicsWorld::step(float dt) {
    b2World_Step(impl_->id, dt, 4);
    impl_->overlaps.clear();
    const b2SensorEvents events = b2World_GetSensorEvents(impl_->id);
    for (int i = 0; i < events.beginCount; ++i) {
        const b2SensorBeginTouchEvent& e = events.beginEvents[i];
        if (!b2Shape_IsValid(e.sensorShapeId) || !b2Shape_IsValid(e.visitorShapeId)) continue;
        impl_->overlaps.push_back({b2Shape_GetUserData(e.sensorShapeId),
                                   b2Shape_GetUserData(e.visitorShapeId)});
    }
}

const std::vector<PhysicsWorld::Overlap>& PhysicsWorld::overlaps() const { return impl_->overlaps; }

}  // namespace xe
