#pragma once

#include "Rigidbody.h"

void Log(const string& message) {
	cout << message << '\n';
}

void LogError(const string& message) {
	cerr << message << '\n';
}

class Component
{
public:
	static inline lua_State* game;

	static inline vector<shared_ptr<luabridge::LuaRef>> onStart;
	static inline vector<shared_ptr<luabridge::LuaRef>> onUpdate;
	static inline vector<shared_ptr<luabridge::LuaRef>> onLateUpdate;

	void init() {
		game = luaL_newstate();
		luaL_openlibs(game);
	}

	shared_ptr<luabridge::LuaRef> load(string name, string scene) {
		if (name == "Rigidbody") return createRigidbody(nullptr);
		else {
			string file = "resources/" + scene + "/resources/component_types/" + name + ".lua";
			//cout << "found in " << file << '\n';
			if (!filesystem::exists(file)) {
				//cout << "nothing found in " << file << '\n';
				file = "resources/component_types/" + name + ".lua";
				if (!filesystem::exists(file)) {
					cout << "error: failed to locate component " << name;
					exit(0);
				}
			}
			if (luaL_dofile(game, file.c_str()) != 0) {
				cout << "problem with lua file " << name;
				exit(0);
			}
			luabridge::LuaRef parent = luabridge::getGlobal(game, name.c_str());
			luabridge::LuaRef instance = luabridge::newTable(game);
			EstablishInheritance(instance, parent);
			return make_shared<luabridge::LuaRef>(instance);
		}
	}

	void EstablishInheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table) {
		luabridge::LuaRef new_metatable = luabridge::newTable(game);
		new_metatable["__index"] = parent_table;

		instance_table.push(game);
		new_metatable.push(game);
		lua_setmetatable(game, -2);
		lua_pop(game, 1);
	}

	static void ReportError(const string& actor_name, const luabridge::LuaException& e)
	{
		string error_message = e.what();

		replace(error_message.begin(), error_message.end(), '\\', '/');

		cout << "\033[31m" << actor_name << " : " << error_message << "\033[0m" << '\n';
	}

	static lua_State* GetLuaState() {
		return game;
	}

	static void addFunctions(shared_ptr<luabridge::LuaRef> component) {
		if ((*component)["OnStart"].isFunction()) onStart.push_back(component);
		if ((*component)["OnUpdate"].isFunction()) onUpdate.push_back(component);
		if ((*component)["OnLateUpdate"].isFunction()) onLateUpdate.push_back(component);
	}

	static bool check(shared_ptr<luabridge::LuaRef> a, shared_ptr<luabridge::LuaRef> b) {
		if ((*a)["actorID"] == (*b)["actorID"]) return (*a)["key"] < (*b)["key"];
		else return (*a)["actorID"] < (*b)["actorID"];
	}

	static shared_ptr<luabridge::LuaRef> createRigidbody(shared_ptr<luabridge::LuaRef> templated) {
		Rigidbody* rigidbody = new Rigidbody();
		Rigidbody& rigid = *rigidbody;
		if (templated != nullptr) {
			rigid.x = (*templated)["x"];
			rigid.y = (*templated)["y"];
			string body_type = (*templated)["body_type"];
			rigid.body_type = body_type;
			rigid.precise = (*templated)["precise"];
			rigid.gravity_scale = (*templated)["gravity_scale"];
			rigid.density = (*templated)["density"];
			rigid.angular_friction = (*templated)["angular_friction"];
			rigid.rotation = (*templated)["rotation"];
			rigid.has_collider = (*templated)["has_collider"];
			rigid.has_trigger = (*templated)["has_trigger"];
			string collider_type = (*templated)["collider_type"];
			rigid.collider_type = collider_type;
			rigid.width = (*templated)["width"];
			rigid.height = (*templated)["height"];
			rigid.radius = (*templated)["radius"];
			rigid.friction = (*templated)["friction"];
			rigid.bounciness = (*templated)["bounciness"];
			string trigger_type = (*templated)["trigger_type"];
			rigid.trigger_type = trigger_type;
			rigid.trigger_width = (*templated)["trigger_width"];
			rigid.trigger_height = (*templated)["trigger_height"];
			rigid.trigger_radius = (*templated)["trigger_radius"];
		}
		luabridge::LuaRef componentRef(game, rigidbody);
		
		return make_shared<luabridge::LuaRef>(componentRef);
	}



};

