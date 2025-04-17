module;

#include <box2d/box2d.h>

export module vis.physic;

import std;
import vis.math;
import vis.chrono;
import vis.ecs;

// non exported
namespace vis::physics {

b2Vec2 to_box2d(vec2 v) {
	return {v.x, v.y};
}

vec2 from_box2d(b2Vec2 v) {
	return {v.x, v.y};
}

} // namespace vis::physics

export namespace vis::physics {

class World;
class ShapeDef;
class Polygon;
struct Circle;
class RigidBody;

struct Rotation {
	float cos_angle, sin_angle;
};

struct Transform {
	vec2 position{};
	Rotation rotation{};
	vec2 scale{1.0f, 1.0f};
};

class SensorBeginTouchEvent {
public:
	SensorBeginTouchEvent() = default;

	[[nodiscard]] RigidBody* get_sensor_body() const;
	[[nodiscard]] RigidBody* get_visitor_body() const;
	[[nodiscard]] ecs::entity get_sensor_entity() const;
	[[nodiscard]] ecs::entity get_visitor_entity() const;

private:
	friend class SensorEvent;
	explicit SensorBeginTouchEvent(const b2SensorBeginTouchEvent& event) : event{event} {}

private:
	b2SensorBeginTouchEvent event;
};

class SensorEndTouchEvent {
public:
	SensorEndTouchEvent() = default;

	[[nodiscard]] RigidBody* get_sensor_body() const;
	[[nodiscard]] RigidBody* get_visitor_body() const;
	[[nodiscard]] ecs::entity get_sensor_entity() const;
	[[nodiscard]] ecs::entity get_visitor_entity() const;

private:
	friend class SensorEvent;
	explicit SensorEndTouchEvent(const b2SensorEndTouchEvent& event) : event{event} {}

private:
	b2SensorEndTouchEvent event;
};

class SensorEvent {
public:
	using SensorBeginTouchVector = std::vector<SensorBeginTouchEvent>;
	using SensorEndTouchVector = std::vector<SensorEndTouchEvent>;

	using SensorBeginTouchVectorValueType = SensorBeginTouchVector::value_type;
	using SensorBeginTouchVectorIterator = SensorBeginTouchVector::iterator;
	using SensorBeginTouchVectorConstIterator = SensorBeginTouchVector::const_iterator;
	using SensorEndTouchVectorValueType = SensorEndTouchVector::value_type;
	using SensorEndTouchVectorIterator = SensorEndTouchVector::iterator;
	using SensorEndTouchVectorConstIterator = SensorEndTouchVector::const_iterator;

	[[nodiscard]] SensorBeginTouchVectorConstIterator begin_begin_touch() const {
		return begin(begin_touch_events);
	}

	[[nodiscard]] SensorBeginTouchVectorConstIterator end_begin_touch() const {
		return end(begin_touch_events);
	}

	[[nodiscard]] SensorEndTouchVectorConstIterator begin_end_touch() const {
		return begin(end_touch_events);
	}

	[[nodiscard]] SensorEndTouchVectorConstIterator end_end_touch() const {
		return end(end_touch_events);
	}

private:
	friend class World;
	explicit SensorEvent(const World& world);

	SensorBeginTouchVector begin_touch_events;
	SensorEndTouchVector end_touch_events;
};

class ContactBeginTouchEvent {
public:
	ContactBeginTouchEvent() = default;

	[[nodiscard]] RigidBody* get_body_a() const;
	[[nodiscard]] RigidBody* get_body_b() const;
	[[nodiscard]] ecs::entity get_entity_a() const;
	[[nodiscard]] ecs::entity get_entity_b() const;

private:
	friend class ContactEvent;
	explicit ContactBeginTouchEvent(const b2ContactBeginTouchEvent& event) : event{event} {}

private:
	b2ContactBeginTouchEvent event;
};

class ContactEndTouchEvent {
public:
	ContactEndTouchEvent() = default;

	[[nodiscard]] RigidBody* get_body_a() const;
	[[nodiscard]] RigidBody* get_body_b() const;
	[[nodiscard]] ecs::entity get_entity_a() const;
	[[nodiscard]] ecs::entity get_entity_b() const;

private:
	friend class ContactEvent;
	explicit ContactEndTouchEvent(const b2ContactEndTouchEvent& event) : event{event} {}

private:
	b2ContactEndTouchEvent event;
};

class ContactEvent {
public:
	using ContactBeginTouchVector = std::vector<ContactBeginTouchEvent>;
	using ContactEndTouchVector = std::vector<ContactEndTouchEvent>;

	using ContactBeginTouchVectorConstIterator = ContactBeginTouchVector::const_iterator;
	using ContactEndTouchVectorConstIterator = ContactEndTouchVector::const_iterator;

	[[nodiscard]] ContactBeginTouchVectorConstIterator begin_begin_touch() const {
		return begin(begin_touch_events);
	}

	[[nodiscard]] ContactBeginTouchVectorConstIterator end_begin_touch() const {
		return end(begin_touch_events);
	}

	[[nodiscard]] ContactEndTouchVectorConstIterator begin_end_touch() const {
		return begin(end_touch_events);
	}

	[[nodiscard]] ContactEndTouchVectorConstIterator end_end_touch() const {
		return end(end_touch_events);
	}

private:
	friend class World;
	explicit ContactEvent(const World& world);

	ContactBeginTouchVector begin_touch_events;
	ContactEndTouchVector end_touch_events;
};

class ContactHitEvent {
public:
	ContactHitEvent() = default;

	[[nodiscard]] RigidBody* body_a() const;
	[[nodiscard]] RigidBody* body_b() const;

	[[nodiscard]] ecs::entity entity_a() const;
	[[nodiscard]] ecs::entity entity_b() const;

	[[nodiscard]] vec2 position() const;
	[[nodiscard]] vec2 normal() const;

private:
	friend class ContactHitEvents;
	explicit ContactHitEvent(const b2ContactHitEvent& event) : event{event} {}

private:
	b2ContactHitEvent event;
};

class ContactHitEvents {
public:
	using ContactHitEventVector = std::vector<ContactHitEvent>;

	using value_type = ContactHitEventVector::value_type;
	using iterator = ContactHitEventVector::iterator;
	using const_iterator = ContactHitEventVector::const_iterator;

	[[nodiscard]] const_iterator begin() const {
		return ::std::begin(events);
	}

	[[nodiscard]] const_iterator end() const {
		return ::std::end(events);
	}

private:
	friend class World;
	explicit ContactHitEvents(const World& world);

private:
	ContactHitEventVector events;
};

enum class BodyType {
	fixed = 0,
	kinematic = 1,
	dynamic = 2,
};

class RigidBodyDef {
public:
	RigidBodyDef() : def{::b2DefaultBodyDef()} {}

	RigidBodyDef& set_body_type(BodyType type) {
		def.type = static_cast<b2BodyType>(type);
		return *this;
	}

	RigidBodyDef& set_transform(const Transform& t) {
		def.position = to_box2d(t.position);
		def.rotation = b2Rot{.c = t.rotation.cos_angle, .s = t.rotation.sin_angle};
		return *this;
	}

	RigidBodyDef& set_position(vec2 pos) {
		def.position = to_box2d(pos);
		return *this;
	}

	RigidBodyDef& set_fixed_rotation(bool is_fixed_rotation) {
		def.fixedRotation = is_fixed_rotation;
		return *this;
	}

	RigidBodyDef& set_linear_velocity(vec2 vel) {
		def.linearVelocity = to_box2d(vel);
		return *this;
	}

	RigidBodyDef& set_rotation(Rotation rot) {
		def.rotation = {.c = rot.cos_angle, .s = rot.sin_angle};
		return *this;
	}

	RigidBodyDef& set_is_bullet(bool is_bullet) {
		def.isBullet = is_bullet;
		return *this;
	}

	explicit operator const b2BodyDef*() const {
		return &def;
	}

private:
	::b2BodyDef def;
};

class RigidBody {
	struct InternalUserData {
		RigidBody* self;
		ecs::entity entity;
	};

public:
	friend class World;
	friend class SensorBeginTouchEvent;
	friend class SensorEndTouchEvent;
	friend class ContactBeginTouchEvent;
	friend class ContactEndTouchEvent;
	friend class ContactHitEvents;
	friend class ContactHitEvent;

	RigidBody(RigidBody&) = delete;
	RigidBody& operator=(RigidBody&) = delete;

	// TODO: can I apply swap idiom here?
	RigidBody(RigidBody&& rhs) noexcept : id{rhs.id}, user_data{.self = this, .entity = rhs.user_data.entity} {
		b2Body_SetUserData(id, &user_data);

		rhs.id = b2_nullBodyId;
		rhs.user_data = InternalUserData{.self = nullptr, .entity = {}};
	}

	// TODO: can I apply swap idiom here?
	RigidBody& operator=(RigidBody&& rhs) noexcept {
		id = rhs.id;
		user_data = InternalUserData{.self = this, .entity = rhs.user_data.entity};
		b2Body_SetUserData(id, &user_data);

		rhs.id = b2_nullBodyId;
		rhs.user_data = InternalUserData{.self = nullptr, .entity = {}};

		return *this;
	}

	~RigidBody() {
		if (b2Body_IsValid(id)) {
			b2DestroyBody(id);
			id = b2_nullBodyId;
		}
	}

	[[nodiscard]] mat4 get_model() const {
		const auto t = get_transform();
		auto model = ext::identity<mat4>();
		model[0][0] = t.rotation.cos_angle;
		model[1][0] = -t.rotation.sin_angle;
		model[0][1] = t.rotation.sin_angle;
		model[1][1] = t.rotation.cos_angle;
		model[3][0] = t.position.x;
		model[3][1] = t.position.y;
		return model;
	}

	RigidBody& create_shape(const ShapeDef& shape, const Polygon& polygon);
	RigidBody& create_shape(const ShapeDef& shape, const Circle& circle);

	[[nodiscard]] Transform get_transform() const {
		Transform res;
		const auto& [p, q] = b2Body_GetTransform(id);
		res.position = from_box2d(p);
		res.rotation = {q.c, q.s};
		return res;
	}

	[[nodiscard]] vec2 get_linear_velocity() const {
		return from_box2d(b2Body_GetLinearVelocity(id));
	}

	RigidBody& set_linear_velocity(vec2 vel) {
		b2Body_SetLinearVelocity(id, to_box2d(vel));
		return *this;
	}

	RigidBody& apply_force_to_center(vec2 force) {
		b2Body_ApplyForceToCenter(id, to_box2d(force), true);
		return *this;
	}

	RigidBody& apply_linear_impulse_to_center(vec2 force) {
		b2Body_ApplyLinearImpulseToCenter(id, to_box2d(force), true);
		return *this;
	}

	RigidBody& set_transform(const Transform& transformation) {
		b2Body_SetTransform(id, to_box2d(transformation.position),
												b2Rot{transformation.rotation.cos_angle, transformation.rotation.sin_angle});
		return *this;
	}

	[[nodiscard]] ecs::entity get_entity() const {
		return user_data.entity;
	}

	RigidBody& set_enable_hit_events(bool enable) {
		b2Body_EnableHitEvents(id, enable);
		return *this;
	}

	explicit operator b2BodyId() const {
		return id;
	}

private:
	RigidBody(const World& world, const RigidBodyDef& def, ecs::entity entity);
	::b2BodyId id;
	InternalUserData user_data;
};

//
// SensorBeginTouchEvent
//
RigidBody* SensorBeginTouchEvent::get_sensor_body() const {
	auto shape = event.sensorShapeId;
	auto body = b2Shape_GetBody(shape);
	return static_cast<RigidBody::InternalUserData*>(b2Body_GetUserData(body))->self;
}

RigidBody* SensorBeginTouchEvent::get_visitor_body() const {
	auto shape = event.visitorShapeId;
	auto body = b2Shape_GetBody(shape);
	return static_cast<RigidBody::InternalUserData*>(b2Body_GetUserData(body))->self;
}

ecs::entity SensorBeginTouchEvent::get_sensor_entity() const {
	return get_sensor_body()->get_entity();
}

ecs::entity SensorBeginTouchEvent::get_visitor_entity() const {
	return get_visitor_body()->get_entity();
}

//
// SensorEndTouchEvent
//
RigidBody* SensorEndTouchEvent::get_sensor_body() const {
	auto shape = event.sensorShapeId;
	auto body = b2Shape_GetBody(shape);
	return static_cast<RigidBody::InternalUserData*>(b2Body_GetUserData(body))->self;
}

RigidBody* SensorEndTouchEvent::get_visitor_body() const {
	auto shape = event.visitorShapeId;
	auto body = b2Shape_GetBody(shape);
	return static_cast<RigidBody::InternalUserData*>(b2Body_GetUserData(body))->self;
}

ecs::entity SensorEndTouchEvent::get_sensor_entity() const {
	return get_sensor_body()->get_entity();
}

ecs::entity SensorEndTouchEvent::get_visitor_entity() const {
	return get_visitor_body()->get_entity();
}

//
// ContactBeginTouchEvent
//
RigidBody* ContactBeginTouchEvent::get_body_a() const {
	auto shape = event.shapeIdA;
	auto body = b2Shape_GetBody(shape);
	return static_cast<RigidBody::InternalUserData*>(b2Body_GetUserData(body))->self;
}

RigidBody* ContactBeginTouchEvent::get_body_b() const {
	auto shape = event.shapeIdB;
	auto body = b2Shape_GetBody(shape);
	return static_cast<RigidBody::InternalUserData*>(b2Body_GetUserData(body))->self;
}

ecs::entity ContactBeginTouchEvent::get_entity_a() const {
	return get_body_a()->get_entity();
}

ecs::entity ContactBeginTouchEvent::get_entity_b() const {
	return get_body_b()->get_entity();
}

//
// ContactEndTouchEvent
//
RigidBody* ContactEndTouchEvent::get_body_a() const {
	auto shape = event.shapeIdA;
	auto body = b2Shape_GetBody(shape);
	return static_cast<RigidBody::InternalUserData*>(b2Body_GetUserData(body))->self;
}

RigidBody* ContactEndTouchEvent::get_body_b() const {
	auto shape = event.shapeIdB;
	auto body = b2Shape_GetBody(shape);
	return static_cast<RigidBody::InternalUserData*>(b2Body_GetUserData(body))->self;
}

ecs::entity ContactEndTouchEvent::get_entity_a() const {
	return get_body_a()->get_entity();
}

ecs::entity ContactEndTouchEvent::get_entity_b() const {
	return get_body_b()->get_entity();
}

//
// ContactHitEvent
//
RigidBody* ContactHitEvent::body_a() const {
	auto shape = event.shapeIdA;
	auto body = b2Shape_GetBody(shape);
	return static_cast<RigidBody::InternalUserData*>(b2Body_GetUserData(body))->self;
}

RigidBody* ContactHitEvent::body_b() const {
	auto shape = event.shapeIdB;
	auto body = b2Shape_GetBody(shape);
	return static_cast<RigidBody::InternalUserData*>(b2Body_GetUserData(body))->self;
}

ecs::entity ContactHitEvent::entity_a() const {
	return body_a()->get_entity();
}

ecs::entity ContactHitEvent::entity_b() const {
	return body_b()->get_entity();
}

vec2 ContactHitEvent::position() const {
	return from_box2d(event.point);
}

vec2 ContactHitEvent::normal() const {
	return from_box2d(event.normal);
}

class PrismaticJointDef {
public:
	PrismaticJointDef() : def{::b2DefaultPrismaticJointDef()} {}

	PrismaticJointDef& set_body_a(const RigidBody& rb) {
		def.bodyIdA = static_cast<b2BodyId>(rb);
		return *this;
	}

	PrismaticJointDef& set_body_b(const vec2& axe) {
		def.localAxisA = to_box2d(axe);
		return *this;
	}

	PrismaticJointDef& enable_limit(bool e) {
		def.enableLimit = e;
		return *this;
	}

	PrismaticJointDef& lower_translation(float val) {
		def.lowerTranslation = val;
		return *this;
	}

	PrismaticJointDef& upper_translation(float val) {
		def.upperTranslation = val;
		return *this;
	}

	explicit operator const b2PrismaticJointDef*() const {
		return &def;
	}

private:
	b2PrismaticJointDef def;
};

class b2PrismaticJoint {
	friend class World;

	explicit operator b2JointId() const {
		return id;
	}

private:
	b2PrismaticJoint(const World& world, const PrismaticJointDef& def);

	::b2JointId id;
};

class WorldDef {
public:
	WorldDef() : def{::b2DefaultWorldDef()} {
		def.restitutionThreshold = 1.0f;
	}

	void set_gravity(vec2 g) {
		def.gravity = to_box2d(g);
	}

	explicit operator const b2WorldDef*() const {
		return &def;
	}

private:
	b2WorldDef def;
};

struct RayCastResult {
	std::reference_wrapper<RigidBody> body;
	vec2 position;
	vec2 normal;
};

class World {
public:
	World() : id{b2_nullWorldId} {}

	friend World create_world(const WorldDef& world_def);

	friend void swap(World& lhs, World& rhs) {
		std::swap(lhs.id, rhs.id);
	}

	World(const World&) = delete;
	World operator=(const World&) = delete;

	World(World&& rhs) noexcept : id{b2_nullWorldId} {
		swap(*this, rhs);
	}

	World& operator=(World&& rhs) noexcept {
		swap(*this, rhs);
		return *this;
	}

	~World() {
		if (b2World_IsValid(id)) {
			b2DestroyWorld(id);
			id = b2_nullWorldId;
		}
	}

	void step(chrono::seconds time_step, int sub_step_count) const {
		b2World_Step(id, time_step.count(), sub_step_count);
	}

	[[nodiscard]] RigidBody create_body(const RigidBodyDef& def, ecs::entity entity) const {
		return RigidBody{*this, def, entity};
	}

	[[nodiscard]] std::optional<RayCastResult> cast_ray(vec2 start, vec2 end) const;

	[[nodiscard]] ContactHitEvents get_hit_events() const {
		return ContactHitEvents{*this};
	}

	[[nodiscard]] SensorEvent get_sensor_events() const {
		return SensorEvent{*this};
	}

	[[nodiscard]] ContactEvent get_contact_events() const {
		return ContactEvent{*this};
	}

	explicit operator b2WorldId() const {
		return id;
	}

private:
	explicit World(const WorldDef& world_def) : id{b2CreateWorld(static_cast<const b2WorldDef*>(world_def))} {}

private:
	b2WorldId id;
};

World create_world(const WorldDef& world_def) {
	return World{world_def};
}

RigidBody::RigidBody(const World& world, const RigidBodyDef& def, ecs::entity entity)
		: id{b2CreateBody(static_cast<b2WorldId>(world), static_cast<const b2BodyDef*>(def))},
			user_data{.self = this, .entity = entity} {
	b2Body_SetUserData(id, &user_data);
}

SensorEvent::SensorEvent(const World& world) {
	auto [beginEvents, endEvents, beginCount, endCount] = b2World_GetSensorEvents(static_cast<b2WorldId>(world));

	begin_touch_events.resize(static_cast<std::size_t>(beginCount));
	end_touch_events.resize(static_cast<std::size_t>(endCount));

	std::transform(beginEvents, beginEvents + beginCount,
								 begin(begin_touch_events), //
								 [](const auto& e) { return SensorBeginTouchEvent{e}; });

	std::transform(endEvents, endEvents + endCount,
								 begin(end_touch_events), //
								 [](const auto& e) { return SensorEndTouchEvent{e}; });
}

ContactEvent::ContactEvent(const World& world) {
	auto [beginEvents, endEvents, hitEvents, beginCount, endCount, hitCount] =
			b2World_GetContactEvents(static_cast<b2WorldId>(world));

	begin_touch_events.resize(static_cast<std::size_t>(beginCount));
	end_touch_events.resize(static_cast<std::size_t>(endCount));

	std::transform(beginEvents, beginEvents + beginCount,
								 begin(begin_touch_events), //
								 [](const auto& e) { return ContactBeginTouchEvent{e}; });

	std::transform(endEvents, endEvents + endCount,
								 begin(end_touch_events), //
								 [](const auto& e) { return ContactEndTouchEvent{e}; });
}

ContactHitEvents::ContactHitEvents(const World& world) {
	auto contacts = b2World_GetContactEvents(static_cast<b2WorldId>(world));

	events.resize(static_cast<std::size_t>(contacts.hitCount));

	std::transform(contacts.hitEvents, contacts.hitEvents + contacts.hitCount,
								 ::std::begin(events), //
								 [](const auto& e) { return ContactHitEvent{e}; });
}

class Polygon {
public:
	friend Polygon create_box2d(vec2 half_extent);

	explicit operator const b2Polygon*() const {
		return &poly;
	}

private:
	explicit Polygon(b2Polygon poly) : poly{poly} {}

private:
	b2Polygon poly;
};

class ShapeDef {
public:
	ShapeDef() : def{b2DefaultShapeDef()} {
		def.userData = this;
	}

	ShapeDef& set_restitution(float restitution) {
		def.restitution = restitution;
		return *this;
	}

	ShapeDef& set_friction(float friction) {
		def.friction = friction;
		return *this;
	}

	ShapeDef& set_is_sensor(bool is_sensor) {
		def.isSensor = is_sensor;
		return *this;
	}

	ShapeDef enable_hit_events(bool enable) {
		def.enableHitEvents = enable;
		return *this;
	}

	ShapeDef enable_contact_events(bool enable) {
		def.enableContactEvents = enable;
		return *this;
	}

	explicit operator const b2ShapeDef*() const {
		return &def;
	}

private:
	b2ShapeDef def;
};

Polygon create_box2d(vec2 half_extent) {
	auto poly = ::b2MakeBox(half_extent.x, half_extent.y);
	return Polygon{poly};
}

struct Circle {
	vec2 center{};
	float radius{};

	explicit operator const b2Circle*() const {
		return reinterpret_cast<const b2Circle*>(this);
	}
};

RigidBody& RigidBody::create_shape(const ShapeDef& shape, const Polygon& polygon) {
	b2CreatePolygonShape(id, static_cast<const b2ShapeDef*>(shape), static_cast<const b2Polygon*>(polygon));
	return *this;
}

RigidBody& RigidBody::create_shape(const ShapeDef& shape, const Circle& circle) {
	b2CreateCircleShape(id, static_cast<const b2ShapeDef*>(shape), static_cast<const b2Circle*>(circle));
	return *this;
}

std::optional<RayCastResult> World::cast_ray(vec2 start, vec2 end) const {
	auto translation = end - start;
	auto res = b2World_CastRayClosest(id, to_box2d(start), to_box2d(translation), b2DefaultQueryFilter());
	if (res.hit == false)
		return std::nullopt;

	auto bodyId = b2Shape_GetBody(res.shapeId);
	auto body = static_cast<RigidBody*>(b2Body_GetUserData(bodyId))->user_data.self;

	return RayCastResult{
			.body = std::ref(*body),
			.position = from_box2d(res.point),
			.normal = from_box2d(res.normal),
	};
}

} // namespace vis::physics