#pragma once
#include "Scene.h"

class Audio
{
public:
    Mix_Chunk* intro_bgm = nullptr;
	bool intro_playing = false;

	static inline unordered_map<string, Mix_Chunk*> audio_db;

	void init(rapidjson::Document *doc) {
		Mix_Init(MIX_INIT_OGG);
		AudioHelper::Mix_OpenAudio498(48000, AUDIO_F32SYS, 2, 2048);
		AudioHelper::Mix_AllocateChannels498(50);
	}

	static Mix_Chunk* loadAudio(string audio) {
		if (audio_db[audio] == nullptr) {
			string wav = "resources/audio/" + audio + ".wav";
			string ogg = "resources/audio/" + audio + ".ogg";
			if (filesystem::exists(wav)) audio_db[audio] = AudioHelper::Mix_LoadWAV498(wav.c_str());
			else if (filesystem::exists(ogg)) audio_db[audio] = AudioHelper::Mix_LoadWAV498(ogg.c_str());
			else {
				cout << "error: failed to play " << audio;
				exit(0);
			}
		}
		return audio_db[audio];
	}

	static void play(int channel, string audio, bool does_loop) {
		if (audio == "") return;
		int loop = (does_loop) ? -1 : 0;
		AudioHelper::Mix_PlayChannel498(channel, loadAudio(audio), loop);
	}


};

