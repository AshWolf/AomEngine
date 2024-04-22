#pragma once

#include "Input.h"

class Scene
{
public:
	static inline rapidjson::Document json;
	static inline vector<Actor*> actors;
	static inline vector<Actor*> backup_actors;
	static inline string name;
	static inline string new_name = "";
	static inline vector<shared_ptr<luabridge::LuaRef>> destroyed;

	static void loadScene() {
		string filepath = "resources/" + name + "/resources/scenes/basic.scene";	
		if (!filesystem::exists(filepath)) {
			filepath = "resources/scenes/" + name + ".scene";
			if (!filesystem::exists(filepath)) {
				cout << "error: scene " << name << " is missing";
				exit(0);
			}
		}
		ReadJsonFile(filepath, json);
		int size = json["actors"].GetArray().Size();
		actors.reserve(size);
		for (const auto& current : json["actors"].GetArray()) {
			Actor* arrival = new Actor;
			arrival->init(current, name);
			actors.emplace_back(arrival);
			for (const auto& component : arrival->components) {
				Component::addFunctions(component.second);
			}
		}
		return;
	}

	static void reset() {
		actors.back()->templates.clear();
		backup_actors.clear();
		Component::onStart.clear();
		Component::onUpdate.clear();
		Component::onLateUpdate.clear();
		for (Actor* actor : actors) {
			if (!actor->destroy) {
				backup_actors.push_back(actor);
				for (const auto& component : actor->components) {
					if ((*component.second)["OnUpdate"].isFunction()) Component::onUpdate.push_back(component.second);
					if ((*component.second)["OnLateUpdate"].isFunction()) Component::onLateUpdate.push_back(component.second);
				}
			}
		}
		for (Actor* current : actors) {
			if (current->destroy) {
				Destroy(current);
				//delete current;
			}
		}
		actors = backup_actors;
	}

	static void swap() {
		name = new_name;
		new_name = "";
		reset();
		loadScene();
	}

	//ENGINE LUA API
	static Actor* Find(const string& name) {
		for (Actor* actor : actors) {
			if (actor->actor_name == name) {
				return actor;
			}
		}
		return luabridge::LuaRef(Component::GetLuaState());
	}
	
	static luabridge::LuaRef FindAll(const string& name) {
		int i = 1;
		luabridge::LuaRef table = luabridge::newTable(Renderer::database.game);
		for (Actor* actor : actors) {
			if (actor->actor_name == name) {
				table[i] = actor;
				i++;
			}
		}
		return table;
	}

	static luabridge::LuaRef FindEvery() {
		int i = 1;
		luabridge::LuaRef table = luabridge::newTable(Renderer::database.game);
		for (Actor* actor : actors) {
			table[i] = actor;
			i++;
		}
		return table;
	}

	static Actor* Instantiate(const string& actor_template_name) {
		Actor* arrival = new Actor;
		arrival->inherit(actor_template_name, name);
		arrival->fix();
		actors.emplace_back(arrival);
		for (const auto& component : arrival->components) {
			Component::addFunctions(component.second);
		}
		return arrival;
	}

	static void Destroy(Actor* actor) {
		auto lambda = [actor](Actor* a) {
			return actor->id == a->id;
			};
		auto it = std::find_if(actors.begin(), actors.end(), lambda);
		if (it == actors.end()) return;
		for (const auto& pair : ((*it)->components)) {
			(*pair.second)["enabled"] = false;
			destroyed.push_back(pair.second);
		}
		actors.erase(it);
		return;
	}

	static void Load(const string& scene_name) {
		new_name = scene_name;
		reset();
		loadScene();
	}

	static string GetCurrent() {
		return name;
	}

	static void DontDestroy(Actor* actor) {
		auto lambda = [actor](Actor* a) {
			return actor->id == a->id;
			};
		auto it = std::find_if(actors.begin(), actors.end(), lambda);
		(*it)->destroy = false;
	}
};

