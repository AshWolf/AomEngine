#pragma once

#include "Component.h"

class Subscription {
public:
	bool operator==(const Subscription& other) const {
		return (event_type == other.event_type && component == other.component && function == other.function);
	}

	string event_type;
	luabridge::LuaRef component;
	luabridge::LuaRef function;
};

class Event
{
public:
	static inline map<string, vector<Subscription>> events;
	static inline vector<Subscription> event_waitlist;
	static inline vector<Subscription> no_fly;
	static inline vector<Subscription> no_fly_waitlist;

	static void Publish(string event_type, luabridge::LuaRef event_object) {
		for (auto const& pair : events[event_type]) {
			if (find(no_fly.begin(), no_fly.end(), pair) == no_fly.end()) {
				pair.function(pair.component, event_object);
			}
		}
		return;
	}

	static void Subscribe(string event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
		Subscription group = { event_type, component, function };
		event_waitlist.push_back(group);
	}

	static void Unsubscribe(string event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
		Subscription group = { event_type, component, function };
		no_fly_waitlist.push_back(group);
	}

	static void newsletter() {
		for (auto const& pair : event_waitlist) {
			events[pair.event_type].push_back(pair);
		}
		event_waitlist.clear();
		for (auto const& pair : no_fly_waitlist) {
			no_fly.push_back(pair);
		}
		no_fly_waitlist.clear();
	}
};

