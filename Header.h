#pragma once

//EDGE CASES I'M CURRENTLY GETTING AWAY WITH
//Player having a velocity
//Using floats instead of doubles
//Player provides input when there is no player actor (moves my camera)
//Damage and step sfx being given to non-player actors
//Player SFX disappearing from one scene to the next
//Checks for pixels-only by looking at first call



#include <vector>
#include <unordered_set>
#include <set>
#include <sstream>
#include <optional>
#include <limits>
#include <cmath>
#include <thread>

#include "filereadstream.h"
#include "document.h"
#include "glm.hpp"
#include "Helper.h"
#include "AudioHelper.h"
#include "SDL_ttf.h"

#include "lua.hpp"
#include "LuaBridge.h"

#include "box2d/box2d.h"

#include <Windows.h>

using namespace std;

static void ReadJsonFile(const std::string& path, rapidjson::Document& out_document)
{
	FILE* file_pointer = nullptr;
#ifdef _WIN32
	fopen_s(&file_pointer, path.c_str(), "rb");
#else
	file_pointer = fopen(path.c_str(), "rb");
#endif
	char buffer[65536];
	rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
	out_document.ParseStream(stream);
	std::fclose(file_pointer);

	if (out_document.HasParseError()) {
		rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
		std::cout << "error parsing json at [" << path << "]" << std::endl;
		exit(0);
	}
}

static uint64_t create_composite_key(int x, int y) {

	//cast to ensure the ints become exactly 32 bits in size
	uint32_t ux = static_cast<uint32_t>(x);
	uint32_t uy = static_cast<uint32_t>(y);

	// place x into right 32 bits
	uint64_t result = static_cast<uint64_t>(ux);

	//move x to left 32 bits
	result = result << 32;

	//place y into right 32 bits
	result = result | static_cast<uint64_t>(uy);

	return result;
}

void getStuff(const rapidjson::Value& current, vector<string> strings, vector<void*> pointers, bool isString) {
	for (int i = 0; i < strings.size(); i++) {
		if (current.HasMember(strings[i].c_str())) {
			if (isString) *static_cast<string*>(pointers[i]) = current[strings[i].c_str()].GetString();
			else {
				*static_cast<float*>(pointers[i]) = current[strings[i].c_str()].GetFloat();
			}
		}
	}
}