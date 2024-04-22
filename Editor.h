#pragma once

#include "Raycast.h"

class Editor
{
public:
	static inline vector<string> scene_list;
	static inline vector<string> actor_list;
	static inline vector<string> comp_list;
	static inline vector<string>* current;
	static inline int mode = 0;

	static void init() {
		std::filesystem::path currentDir = std::filesystem::current_path();
		currentDir += "/resources";

		// Iterate through all files in the directory
		int i = 0;
		for (const auto& entry : std::filesystem::directory_iterator(currentDir)) {
			string filename = entry.path().filename().string();
			vector<string> ignore = { "actor_templates", "audio", "component_types", "fonts", "images", "scenes" };
			if (entry.is_directory() && find(ignore.begin(), ignore.end(), filename) == ignore.end()) {
				scene_list.push_back(filename);
			}
		}
		currentDir += "/actor_templates";
		for (const auto& entry : std::filesystem::directory_iterator(currentDir)) {
			string filename = entry.path().filename().string();
			if (entry.is_regular_file()) {
				actor_list.push_back(filename);
			}
		}
		currentDir = filesystem::current_path();
		currentDir += "/resources/component_types";
		for (const auto& entry : std::filesystem::directory_iterator(currentDir)) {
			string filename = entry.path().filename().string();
			if (entry.is_regular_file()) {
				comp_list.push_back(filename);
			}
		}
		for (string folder : scene_list) {
			currentDir = filesystem::current_path();
			currentDir += "/resources/" + folder + "/resources/component_types";
			for (const auto& entry : std::filesystem::directory_iterator(currentDir)) {
				string filename = entry.path().filename().string();
				if (entry.is_regular_file()) {
					comp_list.push_back(folder + ": " + filename);
				}
			}
		}
	}

	static void menu() {
		int i = 1;
		Renderer::TextDraw2("Files", 0, 0, "NotoSans-Regular", 16, 255, 255, 255, 255);
		Renderer::TextDraw2("Actors", 100, 0, "NotoSans-Regular", 16, 255, 255, 255, 255);
		Renderer::TextDraw2("Components", 200, 0, "NotoSans-Regular", 16, 255, 255, 255, 255);
		if (mode == 0) current = &scene_list;
		else if (mode == 1) current = &actor_list;
		else current = &comp_list;
		for (string thing : *current) {
			Renderer::TextDraw2(thing, 0, i * 20, "NotoSans-Regular", 16, 255, 255, 255, 255);
			i++;
		}
		if (Input::GetMouseButtonDown(1)) {
			int pick = Input::GetMousePosition().y / 20;
			if (pick < 1) {
				int xpick = Input::GetMousePosition().x / 100;
				mode = xpick;
			}
			else if (pick < current->size()) {
				if (mode == 0) {
					Scene::name = (*current)[pick - 1];
					cout << Scene::name << '\n';
					Scene::loadScene();
					Helper::frame_number = 0;
				}
				else if (mode == 1) {
					string name = (*current)[pick - 1];
					OpenFile("component_types", name);
				}
				else {
					string name = (*current)[pick - 1];
					OpenFile("component_types", name);
				}
			}
			
		}
		scroll();
		return;
	}

	static void rend() {
		int i = 0;
		for (Actor* actor : Scene::actors) {
			int j = 300;
			string x = "NA";
			string y = "NA";
			for (auto const& pair : actor->components) {
				float green = 255;
				float red = 0;
				string name = (*pair.second)["nameOfTable"];
				if (!(*pair.second)["enabled"]) {
					name = "~" + name + "~";
					green = 0;
					red = 255;
				}
				Renderer::TextDraw2(name, j, i * 20, "NotoSans-Regular", 16, red, green, 0, 255);
				j += name.length() * 10;
			}
			if (actor->components_by_type.count("Transform")) {
				shared_ptr<luabridge::LuaRef> transform = *actor->components_by_type["Transform"].begin();
				float tx = ((*transform)["x"]);
				float ty = ((*transform)["y"]);
				x = to_string(tx);
				y = to_string(ty);
				
			}
			else if (actor->components_by_type.count("Rigidbody")) {
				shared_ptr<luabridge::LuaRef> rigid = *actor->components_by_type["Rigidbody"].begin();
				b2Vec2 pos = (*rigid)["GetPosition"](*rigid);
				x = to_string(pos.x);
				y = to_string(pos.y);
			}
			Renderer::TextDraw2(actor->actor_name + " x: " + x + " y: " + y, 0, i * 20, "NotoSans-Regular", 16, 255, 255, 255, 255);
			i++;
		}
		scroll();
	}

	static void scroll() {
		if (Input::GetMouseScrollDelta() != 0) {
			Renderer::ecam_pos.y += Input::GetMouseScrollDelta() * 10;
		}
	}

	static void OpenFile(string type, string file) {
		string filePaths = "C:\\Users\\ashwo\\OneDrive - Umich\\Desktop\\Mstuff\\GameEngines\\game_engine_andrewht\\resources\\" + type + "\\" + file;
		const wchar_t* filePath = (wstring(filePaths.begin(), filePaths.end())).c_str(); // Replace with your file path


		// Open the file with the default associated program
		HINSTANCE result = ShellExecuteW(nullptr, L"open", filePath, nullptr, nullptr, SW_SHOW);
		cout << result;
		if (result < reinterpret_cast<HINSTANCE>(32)) cout << "Error opening " << file << '\n';
	}

};

