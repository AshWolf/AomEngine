#pragma once

#include "Actor.h"

enum INPUT_STATE {
	FRAME_DOWN = -2,
	DOWN = -1,
	UP = 0,
	FRAME_UP = 1,
	MOUSE = 2,
	MOUSE_MOVE = 3,
};

class Input
{
public:
	static void Init(); // Call before main loop begins.
	static void ProcessEvent(const SDL_Event& event, INPUT_STATE direction, bool mouse) { // Call every frame at start of event loop.
		if (mouse) {
			if (direction == MOUSE) delta = event.wheel.preciseY;
			else if (direction == MOUSE_MOVE) point = glm::vec2{ event.motion.x, event.motion.y };
			else {
				mouse_states[event.button.button - 1] = direction;
			}
		}
		else keyboard_states[event.key.keysym.scancode] = direction;
	}

	static void LateUpdate() {
		for (auto& key : keyboard_states) {
			if (key.second == FRAME_DOWN) key.second = DOWN;
			if (key.second == FRAME_UP) key.second = UP;
		}
		for (auto& key : mouse_states) {
			if (key == FRAME_DOWN) key = DOWN;
			if (key == FRAME_UP) key = UP;
		}
		delta = 0;
	}

	static bool GetKeyPressed(string keycode) {
		bool value = false;
		try {
			value = (keyboard_states[k2s.at(keycode)] <= DOWN);
		}
		catch (const std::out_of_range& e) {
			return false;
		}

		return value;
	}
	static bool GetKeyDown(string keycode) {
		bool value = false;
		try {
			value = (keyboard_states[k2s.at(keycode)] == FRAME_DOWN);
		}
		catch(const std::out_of_range& e) {
			return false;
		}
		return value;
	}
	static bool GetKeyUp(string keycode) {
		bool value = false;
		try {
			value = (keyboard_states[k2s.at(keycode)] == FRAME_UP);
		}
		catch (const std::out_of_range& e) {
			return false;
		}
		return value;
	}
	static glm::vec2 GetMousePosition() {
		return point;
	}
	static bool GetMouseButtonPressed(int button_num) {
		if (button_num < 1 || button_num > 3) return false;
		else return mouse_states[button_num - 1] <= DOWN;
	}
	static bool GetMouseButtonDown(int button_num) {
		if (button_num < 1 || button_num > 3) return false;
		else return mouse_states[button_num - 1] == FRAME_DOWN;
	}
	static bool GetMouseButtonUp(int button_num) {
		if (button_num < 1 || button_num > 3) return false;
		else return mouse_states[button_num - 1] == FRAME_UP;
	}
	static float GetMouseScrollDelta() {
		return delta;
	}

private:												//^,  >,  v,  <
	static inline unordered_map<SDL_Scancode, INPUT_STATE> keyboard_states;
	static inline INPUT_STATE mouse_states[] = { UP, UP, UP };
	static inline float delta;
	static inline glm::vec2 point = glm::vec2(0,0);
	static inline const unordered_map<string, SDL_Scancode> k2s = {
		// Directional (arrow) Keys
		{"up", SDL_SCANCODE_UP},
		{"down", SDL_SCANCODE_DOWN},
		{"right", SDL_SCANCODE_RIGHT},
		{"left", SDL_SCANCODE_LEFT},

		// Misc Keys
		{"escape", SDL_SCANCODE_ESCAPE},

		// Modifier Keys
		{"lshift", SDL_SCANCODE_LSHIFT},
		{"rshift", SDL_SCANCODE_RSHIFT},
		{"lctrl", SDL_SCANCODE_LCTRL},
		{"rctrl", SDL_SCANCODE_RCTRL},
		{"lalt", SDL_SCANCODE_LALT},
		{"ralt", SDL_SCANCODE_RALT},

		// Editing Keys
		{"tab", SDL_SCANCODE_TAB},
		{"return", SDL_SCANCODE_RETURN},
		{"enter", SDL_SCANCODE_RETURN},
		{"backspace", SDL_SCANCODE_BACKSPACE},
		{"delete", SDL_SCANCODE_DELETE},
		{"insert", SDL_SCANCODE_INSERT},

		// Character Keys
		{"space", SDL_SCANCODE_SPACE},
		{"a", SDL_SCANCODE_A},
		{"b", SDL_SCANCODE_B},
		{"c", SDL_SCANCODE_C},
		{"d", SDL_SCANCODE_D},
		{"e", SDL_SCANCODE_E},
		{"f", SDL_SCANCODE_F},
		{"g", SDL_SCANCODE_G},
		{"h", SDL_SCANCODE_H},
		{"i", SDL_SCANCODE_I},
		{"j", SDL_SCANCODE_J},
		{"k", SDL_SCANCODE_K},
		{"l", SDL_SCANCODE_L},
		{"m", SDL_SCANCODE_M},
		{"n", SDL_SCANCODE_N},
		{"o", SDL_SCANCODE_O},
		{"p", SDL_SCANCODE_P},
		{"q", SDL_SCANCODE_Q},
		{"r", SDL_SCANCODE_R},
		{"s", SDL_SCANCODE_S},
		{"t", SDL_SCANCODE_T},
		{"u", SDL_SCANCODE_U},
		{"v", SDL_SCANCODE_V},
		{"w", SDL_SCANCODE_W},
		{"x", SDL_SCANCODE_X},
		{"y", SDL_SCANCODE_Y},
		{"z", SDL_SCANCODE_Z},
		{"0", SDL_SCANCODE_0},
		{"1", SDL_SCANCODE_1},
		{"2", SDL_SCANCODE_2},
		{"3", SDL_SCANCODE_3},
		{"4", SDL_SCANCODE_4},
		{"5", SDL_SCANCODE_5},
		{"6", SDL_SCANCODE_6},
		{"7", SDL_SCANCODE_7},
		{"8", SDL_SCANCODE_8},
		{"9", SDL_SCANCODE_9},
		{"/", SDL_SCANCODE_SLASH},
		{";", SDL_SCANCODE_SEMICOLON},
		{"=", SDL_SCANCODE_EQUALS},
		{"-", SDL_SCANCODE_MINUS},
		{".", SDL_SCANCODE_PERIOD},
		{",", SDL_SCANCODE_COMMA},
		{"[", SDL_SCANCODE_LEFTBRACKET},
		{"]", SDL_SCANCODE_RIGHTBRACKET},
		{"\\", SDL_SCANCODE_BACKSLASH},
		{"'", SDL_SCANCODE_APOSTROPHE}
	};
};


