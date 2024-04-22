#pragma once

#include "Event.h"

class Renderer
{
public:
	rapidjson::Document doc;
	rapidjson::Document cam;
	SDL_Window* window = nullptr;
	static inline SDL_Renderer* renderer;
	SDL_Window* ewindow = nullptr;
	static inline SDL_Renderer* erenderer;
	float camX = 640;
	float camY = 360;
	string game_title = "";
	float r = 255;
	float g = 255;
	float b = 255;
	static inline float zoom_factor = 1.0f;
	static inline float cam_ease = 1.0f;
	float speed = 0.02f;

	static inline glm::vec2 cam_pos = glm::vec2(0, 0);
	static inline glm::vec2 ecam_pos = glm::vec2(0, 0);

	struct Drawing {
		SDL_Texture* texture = nullptr;
		float x = 0.0f;
		float y = 0.0f;
		int r = 255;
		int g = 255;
		int b = 255;
		int a = 255;
		int type = 0;
		int sorting_order = 0;
		int angle = 0;
		float scale_x = 1.0f;
		float scale_y = 1.0f;
		float pivot_x = 0.5f;
		float pivot_y = 0.5f;
		SDL_Renderer* rnd = Renderer::renderer;

		Drawing(SDL_Texture* texture, int x, int y) : texture(texture), x(x), y(y) {}
		Drawing(SDL_Texture* texture, int x, int y, int r, int g, int b, int a, int sorting_order, int type) : texture(texture), x(x), y(y), r(r), g(g), b(b), a(a), sorting_order(sorting_order), type(type) {}
		Drawing(SDL_Texture* texture, float x, float y, int rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, int r, int g, int b, int a, int sorting_order, int type) : texture(texture),
			x(x), y(y), angle(rotation_degrees), scale_x(scale_x), scale_y(scale_y), pivot_x(pivot_x), pivot_y(pivot_y), r(r), g(g), b(b), a(a), sorting_order(sorting_order), type(type) {}

	};

	queue<SDL_Texture*> images;
	queue<SDL_Texture*> texts;
	TTF_Font* default_font = nullptr;

	static inline unordered_map<string, SDL_Texture*> image_db;
	static inline unordered_map<string, SDL_Texture*> text_db;
	static inline unordered_map<string, unordered_map<int, TTF_Font*>> font_db;
	static inline vector<SDL_Texture*> death_note;
	static inline vector<Drawing> render_order;

	static inline Component database;

	void init() {
		if (!filesystem::exists("resources")) {
			cout << "error: resources/ missing";
			exit(0);
		}
		else if (!filesystem::exists("resources/game.config")) {
			cout << "error: resources/game.config missing";
			exit(0);
		}
		ReadJsonFile("resources/game.config", doc);
		if (filesystem::exists("resources/rendering.config")) {
			ReadJsonFile("resources/rendering.config", cam);
			vector<string> cams = { "x_resolution", "y_resolution", "clear_color_r", "clear_color_g", "clear_color_b", "zoom_factor", "cam_ease_factor" };
			vector<void*> cam_p = { &camX, &camY, &r, &g, &b, &zoom_factor, &cam_ease };
			getStuff(cam, cams, cam_p, false);
		}
		if (doc.HasMember("game_title")) game_title = doc["game_title"].GetString();
		TTF_Init();
		if (doc.HasMember("font")) {
			string font = doc["font"].GetString();
			string file = "resources/fonts/" + font + ".ttf";
			if (!filesystem::exists(file)) {
				cout << "error: font " << font << " missing";
				exit(0);
			}
			default_font = TTF_OpenFont(file.c_str(), 16);
		}
		window = Helper::SDL_CreateWindow498(game_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, camX, camY, SDL_WINDOW_SHOWN);
		renderer = Helper::SDL_CreateRenderer498(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
		ewindow = Helper::SDL_CreateWindow498(game_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, camX, camY, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		erenderer = Helper::SDL_CreateRenderer498(ewindow, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
		clear();
	}

	void present() {
		auto sorting = [](Drawing a, Drawing b) {
			if (a.type == b.type) return a.sorting_order < b.sorting_order;
			else return a.type < b.type;
			};
		if (!render_order.empty() && render_order.front().type < 3) stable_sort(render_order.begin(), render_order.end(), sorting);
		SDL_RenderSetScale(renderer, zoom_factor, zoom_factor);
		int i = 0;
		for (; i < render_order.size(); i++) {
			Drawing& d = render_order[i];
			if (d.type == 0) draw(d);
			else {
				SDL_RenderSetScale(renderer, 1, 1);
				break;
			}
		}
		for (; i < render_order.size(); i++) {
			Drawing& d = render_order[i];
			if (d.type < 3) draw(d);
			else {
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
				break;
			}
		}
		int pr = r;
		int pg = g;
		int pb = b;
		int pa = 255;
		for (; i < render_order.size(); i++) {
			Drawing& d = render_order[i];
			if (d.r != pr || d.g != pg || d.b != pb || pa != d.a) {
				SDL_SetRenderDrawColor(renderer, d.r, d.g, d.b, d.a);
				pr = d.r;
				pb = d.b;
				pg = d.g;
				pa = d.a;
			}
			SDL_RenderDrawPoint(renderer, d.x, d.y);
		}
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
		render_order.clear();
		Helper::SDL_RenderPresent498(renderer);
		SDL_RenderPresent(erenderer);
	}

	void clear() {
		SDL_SetRenderDrawColor(renderer, r, g, b, 0);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(erenderer, 0, 0, 0, 0);
		SDL_RenderClear(erenderer);
	}

	static SDL_Texture* generateText(string words, string font, int font_size, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Renderer* rnd) {
		if (text_db[words] == nullptr) {
			SDL_Surface* text;

			SDL_Color color = { r, g, b, a };
			text = TTF_RenderText_Solid(loadFont(font, font_size), words.c_str(), color);
			text_db[words] = SDL_CreateTextureFromSurface(rnd, text);
			SDL_FreeSurface(text);
		}
		return text_db[words];
	}

	static TTF_Font* loadFont(string font, int font_size) {
		if (font_db[font][font_size] == nullptr) {
			string file = "resources/fonts/" + font + ".ttf";
			font_db[font][font_size] = TTF_OpenFont(file.c_str(), font_size);
		}
		return font_db[font][font_size];
	}

	void static displayText(SDL_Texture* text, int x, int y, bool second) {
		SDL_Texture* texture = text;
		Drawing d(texture, x, y, 255, 255, 255, 255, 0, 2);
		if (second) d.rnd = erenderer;
		render_order.push_back(d);
		death_note.emplace_back(texture);
		/*
		int w; int h;
		SDL_QueryTexture(text, NULL, NULL, &w, &h);
		SDL_Rect dest = { x, y, w, h };;
		SDL_RenderCopy(renderer, text, nullptr, &dest);
		death_note.emplace_back(text);
		*/
	}

	static SDL_Texture* loadImage(string name) {
		if (image_db[name] == nullptr) {
			string file = "resources/images/" + name + ".png";
			if (!filesystem::exists(file)) {
				cout << "error: missing image " + name;
				exit(0);
			}
			SDL_Texture* arrival = IMG_LoadTexture(renderer, file.c_str());
			image_db[name] = arrival;
			death_note.emplace_back(arrival);
		}
		return image_db[name];
	}

	void draw(Drawing d) {
		int w; int h;
		SDL_QueryTexture(d.texture, NULL, NULL, &w, &h);
		int flip_mode = SDL_FLIP_NONE;
		if (d.scale_x < 0)
			flip_mode |= SDL_FLIP_HORIZONTAL;
		if (d.scale_y < 0)
			flip_mode |= SDL_FLIP_VERTICAL;
		float x_scale = abs(d.scale_x);
		float y_scale = abs(d.scale_y);

		if (d.type == 0) {
			d.pivot_x = static_cast<int>(d.pivot_x * w * x_scale);
			d.pivot_y = static_cast<int>(d.pivot_y * h * y_scale);
			d.x = (d.x - cam_pos.x) * 100.0f + camX * 0.5f / zoom_factor - d.pivot_x;
			d.y = (d.y - cam_pos.y) * 100.0f + camY * 0.5f / zoom_factor - d.pivot_y;
		}
		if (d.type <= 1) {
			SDL_SetTextureColorMod(d.texture, d.r, d.g, d.b);
			SDL_SetTextureAlphaMod(d.texture, d.a);
		}
		SDL_Rect dest = { static_cast<int>(d.x), static_cast<int>(d.y), static_cast<int>(w * x_scale), static_cast<int>(h * y_scale) };
		SDL_Point center = { static_cast<int>(d.pivot_x), static_cast<int>(d.pivot_y) };
		Helper::SDL_RenderCopyEx498(0, "0", d.rnd, d.texture, nullptr, &dest, d.angle, &center, static_cast<SDL_RendererFlip>(flip_mode));
		SDL_SetTextureColorMod(d.texture, 255, 255, 255);
		SDL_SetTextureAlphaMod(d.texture, 255);
	}

	//LUA ENGINE API
	static void TextDraw(const string& str_content, float x, float y, const string& font_name, float font_size, float r, float g, float b, float a) {
		displayText(generateText(str_content, font_name, font_size, r, g, b, a, renderer), x, y, false);
	}

	static void TextDraw2(const string& str_content, float x, float y, const string& font_name, float font_size, float r, float g, float b, float a) {
		displayText(generateText(str_content, font_name, font_size, r, g, b, a, erenderer), x, y + ecam_pos.y, true);
	}

	static void DrawUI(const string& image_name, float x, float y) {
		DrawUIEx(image_name, x, y, 255, 255, 255, 255, 0);
	}

	static void DrawUIEx(const string& image_name, float x, float y, float r, float g, float b, float a, float sorting_order) {
		SDL_Texture* texture = loadImage(image_name);
		Drawing d(texture, x, y, r, g, b, a, sorting_order, 1);
		render_order.push_back(d);
		death_note.emplace_back(texture);
	}

	static void ImageDraw(const string& image_name, float x, float y) {
		ImageDrawEx(image_name, x, y, 0, 1, 1, 0.5f, 0.5f, 255, 255, 255, 255, 0);
	}

	static void ImageDrawEx(const string& image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order) {
		SDL_Texture* texture = loadImage(image_name);
		Drawing d(texture, x, y, rotation_degrees, scale_x, scale_y, pivot_x, pivot_y, r, g, b, a, sorting_order, 0);
		render_order.push_back(d);
		death_note.emplace_back(texture);
	}

	static void DrawPixel(float x, float y, float r, float g, float b, float a) {
		Drawing d(nullptr, x, y, r, g, b, a, 0, 3);
		render_order.push_back(d);
	}

	static void SetPosition(float x, float y) {
		cam_pos = glm::vec2(x, y);
	}

	static float GetPositionX() {
		return cam_pos.x;
	}

	static float GetPositionY() {
		return cam_pos.y;
	}

	static void SetZoom(float zoom) {
		zoom_factor = zoom;
	}

	static float GetZoom() {
		return zoom_factor;
	}

};

