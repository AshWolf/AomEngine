#pragma once

#include "Audio.h"

class Raycast
{
public:
	class HitResult {
	public:
		Actor* actor;
		b2Vec2 point;
		b2Vec2 normal;
		bool is_trigger;
	};

	static inline vector<HitResult> hits;

	class MyRayCastCallback : public b2RayCastCallback
	{
		float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
			const b2Vec2& normal, float fraction) override {
			Actor* a = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
			if (a == nullptr || fixture->GetFilterData().maskBits == 0) return -1;
			HitResult hit;
			hit.actor = a;
			hit.point = point;
			hit.normal = normal;
			hit.is_trigger = fixture->IsSensor();
			hits.push_back(hit);
			return 1;
		}
	};

	static luabridge::LuaRef RaycastOne(b2Vec2 pos, b2Vec2 dir, float dist) {
		if (dist <= 0) return luabridge::LuaRef(Component::GetLuaState());
		launchRaycast(pos, dir, dist);
		if (hits.empty()) return luabridge::LuaRef(Component::GetLuaState());
		HitResult result = hits.front();
		hits.clear();
		return luabridge::LuaRef(Component::game, result);
	}

	static luabridge::LuaRef RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist) {
		if (dist > 0) launchRaycast(pos, dir, dist);
		int i = 1;
		luabridge::LuaRef table = luabridge::newTable(Component::game);
		for (const HitResult& hit : hits) {
			table[i] = hit;
			i++;
		}
		hits.clear();
		return table;
	}

	static void launchRaycast(b2Vec2 pos, b2Vec2 dir, float dist) {
		MyRayCastCallback callback;
		//dir.Normalize();
		Rigidbody::physics->RayCast(&callback, pos, pos + dist * dir);
		auto lambda = [pos](HitResult a, HitResult b) {
			b2Vec2 loca = a.point;
			b2Vec2 locb = b.point;
			return (loca.x - pos.x + loca.y - pos.y) < (locb.x - pos.x + locb.y - pos.y);
			};
		sort(hits.begin(), hits.end(), lambda);
	}
};

