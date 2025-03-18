module;

#include <box2d/box2d.h>
#include <optional>

export module vis:physic;

import std;
import :math;

export namespace vis::physics {

class World;
class ShapeDef;
class Polygon;
struct Circle;

struct Rotation {
	float cos_angle, sin_angle;
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

	RigidBodyDef& set_position(vis::vec2 pos) {
		def.position = b2Vec2(pos.x, pos.y);
		return *this;
	}
	RigidBodyDef& set_linear_velocity(vis::vec2 vel) {
		def.linearVelocity = b2Vec2(vel.x, vel.y);
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

struct Transformation {
	vec2 position{};
	vec2 scale{1.0f, 1.0f};
	Rotation rotation{};

	mat4 get_model() const {
		auto model = vis::ext::identity<vis::mat4>();
		model[0][0] = rotation.cos_angle;
		model[1][0] = -rotation.sin_angle;
		model[0][1] = rotation.sin_angle;
		model[1][1] = rotation.cos_angle;
		model[3][0] = position.x;
		model[3][1] = position.y;
		return model;
	}
};

class RigidBody {
public:
	friend class World;

	RigidBody(RigidBody&) = delete;
	RigidBody& operator=(RigidBody&) = delete;

	RigidBody(RigidBody&& rhs) : id{rhs.id} {
		rhs.id = b2_nullBodyId;
	}

	RigidBody& operator=(RigidBody&& rhs) {
		id = rhs.id;
		rhs.id = b2_nullBodyId;
		return *this;
	}

	~RigidBody() {
		if (b2Body_IsValid(id)) {
			b2DestroyBody(id);
			id = b2_nullBodyId;
		}
	}

	RigidBody& create_shape(const ShapeDef& shape, const Polygon& polygon);
	RigidBody& create_shape(const ShapeDef& shape, const Circle& circle);

	Transformation get_transform() const {
		Transformation res;
		const auto& [p, q] = b2Body_GetTransform(id);
		res.position = vec2{p.x, p.y};
		res.rotation = {q.c, q.s};
		return res;
	}

	vec2 get_linear_velocity() const {
		auto v = b2Body_GetLinearVelocity(id);
		return vis::vec2(v.x, v.y);
	}

	RigidBody& set_linear_velocity(vis::vec2 vel) {
		b2Body_SetLinearVelocity(id, b2Vec2{vel.x, vel.y});
		return *this;
	}

	RigidBody& set_transform(const Transformation& transformation) {
		b2Body_SetTransform(id, b2Vec2{transformation.position.x, transformation.position.y},
												b2Rot{transformation.rotation.cos_angle, transformation.rotation.sin_angle});
		return *this;
	}

	explicit operator b2BodyId() const {
		return id;
	}

private:
	RigidBody(const World& world, const RigidBodyDef& def);
	::b2BodyId id;
};

class PrismaticJointDef {
public:
	PrismaticJointDef() : def{::b2DefaultPrismaticJointDef()} {}

	PrismaticJointDef& set_body_a(const RigidBody& rb) {
		def.bodyIdA = static_cast<b2BodyId>(rb);
		return *this;
	}

	PrismaticJointDef& set_body_b(const vec2& axe) {
		def.localAxisA = b2Vec2{axe.x, axe.y};
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
		def.gravity = b2Vec2(g.x, g.y);
	}

	explicit operator const b2WorldDef*() const {
		return &def;
	}

private:
	b2WorldDef def;
};

class World {
public:
	friend std::optional<World> create_world(const WorldDef& world_def);

	World(const World&) = delete;
	World operator=(const World&) = delete;

	World(World&& rhs) : id{rhs.id} {
		rhs.id = b2_nullWorldId;
	}

	World& operator=(World&& rhs) {
		id = rhs.id;
		rhs.id = b2_nullWorldId;
		return *this;
	}

	~World() {
		if (b2World_IsValid(id)) {
			b2DestroyWorld(id);
			id = b2_nullWorldId;
		}
	}

	void step(float time_step, int sub_step_count) const {
		b2World_Step(id, time_step, sub_step_count);
	}

	RigidBody create_body(const RigidBodyDef& def) const {
		return RigidBody{*this, def};
	}

	explicit operator b2WorldId() const {
		return id;
	}

private:
	explicit World(const WorldDef& world_def) : id{b2CreateWorld(static_cast<const b2WorldDef*>(world_def))} {}

private:
	b2WorldId id;
};

std::optional<World> create_world(const WorldDef& world_def) {
	auto w = World{world_def};
	return std::optional<World>{std::move(w)};
}

RigidBody::RigidBody(const World& world, const RigidBodyDef& def) {
	id = b2CreateBody(static_cast<b2WorldId>(world), static_cast<const b2BodyDef*>(def));
}

class Polygon {
public:
	friend Polygon create_box2d(vis::vec2 half_extent);

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
	ShapeDef() : def{b2DefaultShapeDef()} {}

	ShapeDef& set_restitution(float restitution) {
		def.restitution = restitution;
		return *this;
	}

	ShapeDef& set_friction(float friction) {
		def.friction = friction;
		return *this;
	}

	explicit operator const b2ShapeDef*() const {
		return &def;
	}

private:
	b2ShapeDef def;
};

Polygon create_box2d(vis::vec2 half_extent) {
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

} // namespace vis::physics