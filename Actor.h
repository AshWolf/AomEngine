#pragma once

#include "Renderer.h"

struct MyComparator {
	bool operator() (const shared_ptr<luabridge::LuaRef>& lhs, const shared_ptr<luabridge::LuaRef>& rhs) const {
		return (*lhs)["key"] < (*rhs)["key"];
	}
};

class Actor
{
public:
	static inline lua_State* game;

	static inline unordered_map<string, shared_ptr<Actor>> templates;
	static inline int count = 0;
	static inline int add_count = 0;
	string actor_name = "";
	string template_name = "";
	int id = 0;
	string scene = "";
	map<string, shared_ptr<luabridge::LuaRef>> components;
	unordered_map<string, set<shared_ptr<luabridge::LuaRef>, MyComparator>> components_by_type;
	bool destroy = true;

	static inline vector<shared_ptr<luabridge::LuaRef>> sortme;


	void init(const rapidjson::Value& current, string name) {
		game = Renderer::database.game;
		scene = name;
		if (current.HasMember("template")) {
			inherit(current["template"].GetString(), scene);
			fix();
		}
		else {
			id = count;
			count++;
		}
		if (current.HasMember("name")) actor_name = (current["name"].GetString());

		if (current.HasMember("components")) {
			const rapidjson::Value& comp_obj = current["components"];
			for (auto it = comp_obj.MemberBegin(); it != comp_obj.MemberEnd(); ++it) {
				string name = it->name.GetString();
				if (it->value.HasMember("type")) {
					shared_ptr<luabridge::LuaRef> latest = adder(name, it->value["type"].GetString());
				}
				for (auto jt = it->value.MemberBegin(); jt != it->value.MemberEnd(); ++jt) {
					string var = jt->name.GetString();
					if (var != "type") {
						shared_ptr<luabridge::LuaRef> latest = components[name];
						if (jt->value.IsString()) (*latest)[var] = jt->value.GetString();
						else if (jt->value.IsBool()) (*latest)[var] = jt->value.GetBool();
						else if (jt->value.IsInt()) (*latest)[var] = jt->value.GetInt();
						else (*latest)[var] = jt->value.GetFloat();
					}
				}
				(*components[name])["actor"] = this;
			}
		}
	}

	void inherit(string temp_name, string override) {
		template_name = temp_name;
		if (templates.find(temp_name) == templates.end()) {
			rapidjson::Document temp;
			if (!filesystem::exists("resources/actor_templates/" + temp_name + ".template")) {
				cout << "error: template " << temp_name << " is missing";
				exit(0);
			}
			ReadJsonFile("resources/actor_templates/" + temp_name + ".template", temp);
			shared_ptr<Actor> templ = make_shared<Actor>();
			templ->init(temp, override);
			templ->id = 69;
			templates[temp_name] = templ;
		}
		actor_name = templates[temp_name]->actor_name;
		id = count;
		count++;
		return;
	}

	void fix() {
		for (const auto& pair : templates[template_name]->components) {
			string name = (*pair.second)["nameOfTable"];
			if (name == "Rigidbody") {
				components[pair.first] = Component::createRigidbody(pair.second);
			}
			else {
				luabridge::LuaRef instance = luabridge::newTable(game);
				luabridge::LuaRef parent = *pair.second;
				Renderer::database.EstablishInheritance(instance, parent);
				components[pair.first] = make_shared<luabridge::LuaRef>(instance);
			}
			(*components[pair.first])["actor"] = this;
			(*components[pair.first])["actorID"] = id;
			components_by_type[(*components[pair.first])["nameOfTable"]].insert(components[pair.first]);
		}
	}

	//ENGINE LUA API
	string GetName() {
		return this->actor_name;
	}

	int GetID() {
		return this->id;
	}

	luabridge::LuaRef GetComponentByKey(const string& key) {
		if (components.count(key) == 0) return luabridge::LuaRef(Component::GetLuaState());
		return *components[key];
	}

	luabridge::LuaRef GetComponent(const string& type) {
		if (components_by_type[type].empty()) {
			return luabridge::LuaRef(Component::GetLuaState());
		}
		else {
			for (const auto& component : components_by_type[type]) {
				if ((*component)["enabled"]) {
					return *component;
				}
			}
		}
		return luabridge::LuaRef(Component::GetLuaState());
	}

	luabridge::LuaRef GetComponents(const string& type) {
		luabridge::LuaRef table = luabridge::newTable(game);
		int i = 1;
		for (const auto& component : components_by_type[type]) {
			if ((*component)["enabled"]) {
				table[i] = *component;
				i++;
			}
		}
		return table;
	}

	luabridge::LuaRef AddComponent(const string& type_name) {
		string name = "r" + to_string(add_count);
		add_count++;
		shared_ptr<luabridge::LuaRef> point = adder(name, type_name);;
		sortme.push_back(point);
		return *point;
	}

	static void AddComponents() {
		sort(sortme.begin(), sortme.end(), [](shared_ptr<luabridge::LuaRef> a, shared_ptr<luabridge::LuaRef> b) {
			if ((*a)["actorID"] == (*b)["actorID"]) return (*a)["key"] < (*b)["key"];
			else return (*a)["actorID"] < (*b)["actorID"];
			});
		for (shared_ptr<luabridge::LuaRef> component : sortme) {
			Component::addFunctions(component);
		}
		sortme.clear();
	}

	shared_ptr<luabridge::LuaRef> adder(string& name, const string& type_name) {
		shared_ptr<luabridge::LuaRef> latest = Renderer::database.load(type_name, scene);
		try {
			(*latest)["key"] = name;
			(*latest)["enabled"] = true;
			(*latest)["nameOfTable"] = type_name;
			(*latest)["actor"] = this;
			(*latest)["actorID"] = id;
			components[name] = latest;
			components_by_type[type_name].insert(latest);
			return latest;
		}
		catch (const luabridge::LuaException& e) {
			cout << e.what() << '\n';
		}
		return nullptr;
	}

	void RemoveComponent(const luabridge::LuaRef& component_ref) {
		component_ref["enabled"] = false;
		string type = component_ref["nameOfTable"];
		if (component_ref["OnDestroy"].isFunction()) {
			component_ref["OnDestroy"](component_ref);
		}
		return;
	}

	Actor() {}
};

bool operator< (const Actor& actor1, const Actor& actor2)
{
	return (actor1.id < actor2.id);
}
