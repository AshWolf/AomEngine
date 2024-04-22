#pragma once

#include "Header.h"

class Actor;

struct Collision {
	Actor* other;
	b2Vec2 point = b2Vec2(-999.0f, -999.0f);
	b2Vec2 relative_velocity;
	b2Vec2 normal = b2Vec2(-999.0f, -999.0f);
};

class Rigidbody
{

public:
	b2Body* body;
	static inline b2World* physics;

	float x = 0.0f;
	float y = 0.0f;
	string body_type = "dynamic";
	bool precise = true;
	float gravity_scale = 1.0f;
	float density = 1.0f;
	float angular_friction = 0.3f;
	float rotation = 0.0f;
	bool has_collider = true;
	bool has_trigger = true;

	string collider_type = "box"; // define the shape of the collider.
	float width = 1.0f;
	float height = 1.0f;
	float radius = 0.5f;
	float friction = 0.3f;
	float bounciness = 0.3f;

	string trigger_type = "box";
	float trigger_width = 1.0f;
	float trigger_height = 1.0f;
	float trigger_radius = 0.5f;
	
	string key = "";
	bool enabled = true;
	string type = "Rigidbody";
	Actor* actor = nullptr;
	int actorID = -1;

	static void initWorld() {
		b2Vec2 gravity(0.0f, -9.8f);
		physics = new b2World(gravity);
	}

	b2Vec2 GetPosition() {
		if (body == nullptr) return b2Vec2(x, y);
		else return body->GetPosition();
	}

	float GetRotation() {
		return body->GetAngle() * (180.0f / b2_pi);
	}

	void OnStart() {
		b2BodyDef def;
		if (body_type == "dynamic") def.type = b2_dynamicBody;
		else if (body_type == "static") def.type = b2_staticBody;
		else def.type = b2_kinematicBody;
		def.position = b2Vec2(x, y);
		def.bullet = precise;
		def.angularDamping = angular_friction;
		def.gravityScale = -gravity_scale;

		body = physics->CreateBody(&def);
		if (!has_collider && !has_trigger) createFixture(false, false);
		if (has_collider) createFixture(true, false);
		if (has_trigger) createFixture(false, true);
		body->SetTransform(b2Vec2(x, y), rotation * (b2_pi / 180.0f));
	}

	void OnDestroy() {
		physics->DestroyBody(body);
	}

	void createFixture(bool collider, bool trigger) {
		b2FixtureDef fixture;
		string type = "box";
		float w = width;
		float h = height;
		float r = 0.0f;

		fixture.density = density;
		if (collider) {
			fixture.friction = friction;
			fixture.restitution = bounciness;
			type = collider_type;
			w = width;
			h = height;
			r = radius;
		}
		else if (trigger) {
			type = trigger_type;
			w = trigger_width;
			h = trigger_height;
			r = trigger_radius;
		}
		else fixture.filter.maskBits = 0;

		b2Shape* my_shape;

		b2PolygonShape box;
		box.SetAsBox(0.5f * w, 0.5f * h);

		b2CircleShape circle;
		circle.m_radius = r;

		if (type == "box") my_shape = &box;
		else my_shape = &circle;

		
		fixture.shape = my_shape;
		fixture.isSensor = !collider;

		fixture.userData.pointer = reinterpret_cast<uintptr_t>(actor);

		body->CreateFixture(&fixture);
	}
	//LUA ENGINE API

	void AddForce(b2Vec2 force) {
		body->ApplyForceToCenter(force, true);
	}

	void SetVelocity(b2Vec2 velocity) {
		body->SetLinearVelocity(velocity);
	}

	void SetPosition(b2Vec2 position) {
		if (body == nullptr) {
			x = position.x;
			y = position.y;
		}
		else body->SetTransform(position, GetRotation());
	}
	
	void SetRotation(float degrees_clockwise) {
		body->SetTransform(GetPosition(), degrees_clockwise * (b2_pi / 180.0f));
	}

	void SetAngularVelocity(float degrees_clockwise) {
		body->SetAngularVelocity(degrees_clockwise * (b2_pi / 180.0f));
	}

	void SetGravityScale(float scale) {
		body->SetGravityScale(-scale);
	}

	void SetUpDirection(b2Vec2 direction) {
		float new_angle_radians = glm::atan(direction.x, -direction.y);
		body->SetTransform(GetPosition(), new_angle_radians);
	}

	void SetRightDirection(b2Vec2 direction) {
		float new_angle_radians = glm::atan(direction.x, -direction.y) - (b2_pi / 2.0f);
		body->SetTransform(GetPosition(), new_angle_radians);
	}

	b2Vec2 GetVelocity() {
		return body->GetLinearVelocity();
	}

	float GetAngularVelocity() {
		return body->GetAngularVelocity() * (180.0f / b2_pi);
	}

	float GetGravityScale() {
		return -body->GetGravityScale();
	}

	b2Vec2 GetUpDirection() {
		float angle = body->GetAngle();
		b2Vec2 up = b2Vec2(glm::sin(angle), -glm::cos(angle));
		up.Normalize();
		return up;
	}

	b2Vec2 GetRightDirection() {
		float angle = body->GetAngle();
		b2Vec2 right = b2Vec2(glm::cos(angle), glm::sin(angle));
		right.Normalize();
		return right;
	}

};

