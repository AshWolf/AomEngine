#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include "Editor.h"


enum GameState {
	Intro = -2,
	Ongoing = 0,
	Good = 1,
	Bad = -1,
	Quit = 2,
};

class AomEngine {
public:
	class Contact : public b2ContactListener {
	public:
		void BeginContact(b2Contact* contact) {
			if (!contact->GetFixtureA()->IsSensor() && !contact->GetFixtureB()->IsSensor()) ContactHelper(contact, "OnCollisionEnter");
			else if (contact->GetFixtureA()->IsSensor() && contact->GetFixtureB()->IsSensor()) ContactHelper(contact, "OnTriggerEnter");
			return;
		}
		void EndContact(b2Contact* contact) {
			if (!contact->GetFixtureA()->IsSensor() && !contact->GetFixtureB()->IsSensor()) ContactHelper(contact, "OnCollisionExit");
			else if (contact->GetFixtureA()->IsSensor() && contact->GetFixtureB()->IsSensor()) ContactHelper(contact, "OnTriggerExit");
			return;
		}

		void ContactHelper(b2Contact* contact, string function) {
			Collision cola;
			Collision colby;
			b2Fixture* a = contact->GetFixtureA();
			b2Fixture* b = contact->GetFixtureB();
			Actor* Aa = reinterpret_cast<Actor*>(a->GetUserData().pointer);
			Actor* Ab = reinterpret_cast<Actor*>(b->GetUserData().pointer);
			if (function == "OnCollisionEnter") {
				b2WorldManifold manifold;
				contact->GetWorldManifold(&manifold);
				cola.point = manifold.points[0];
				cola.normal = manifold.normal;
			}
			cola.relative_velocity = a->GetBody()->GetLinearVelocity() - b->GetBody()->GetLinearVelocity();
			colby = cola;
			cola.other = Ab;
			colby.other = Aa;
			for (auto const& pair : Aa->components) {
				call(function, *pair.second, &cola);
			}
			for (auto const& pair : Ab->components) {
				call(function, *pair.second, &colby);
			}
		}
	};

	static inline Renderer renderer;
	Audio audio;
	Input input;

	Scene scene;
	GameState state = Intro;

	vector <shared_ptr<luabridge::LuaRef>> onStart;
	vector <shared_ptr<luabridge::LuaRef>> onUpdate;
	vector <shared_ptr<luabridge::LuaRef>> onLateUpdate;

	Contact contact;

	bool edit = false;

	int run() {
		renderer.database.init();
		luaInit();
		init();
		if (!renderer.images.size() && !renderer.texts.size()) state = Ongoing; //Skips non-existent intro
		SDL_Event next_event;
		while (state == Ongoing) {
			input.LateUpdate();
			while (Helper::SDL_PollEvent498(&next_event)) {
				switch (next_event.type) {
				case SDL_QUIT:
					state = Quit;
					break;
				case SDL_KEYDOWN:
					input.ProcessEvent(next_event, FRAME_DOWN, false);
					break;
				case SDL_KEYUP:
					input.ProcessEvent(next_event, FRAME_UP, false);
					break;
				case SDL_MOUSEBUTTONDOWN:
					input.ProcessEvent(next_event, FRAME_DOWN, true);
					break;
				case SDL_MOUSEBUTTONUP:
					input.ProcessEvent(next_event, FRAME_UP, true);
					break;
				case SDL_MOUSEWHEEL:
					input.ProcessEvent(next_event, MOUSE, true);
					break;
				case SDL_MOUSEMOTION:
					input.ProcessEvent(next_event, MOUSE_MOVE, true);
					break;
				}
			}
			if (Scene::name.empty()) Editor::menu();
			else {
				collect();
				actorTrigger("OnStart");
				actorTrigger("OnUpdate");
				actorTrigger("OnLateUpdate");
				actorTrigger("OnDestroy");
				Editor::rend();
				scene.destroyed.clear();
				Actor::AddComponents();
				Event::newsletter();
				Rigidbody::physics->Step(1.0f / 60.0f, 8, 3);
			}
			renderer.present();
			renderer.clear();
			if (!scene.new_name.empty()) scene.swap();
		}
		if (state == Quit) safeQuit();
		safeQuit();
		return 0;
	}

	void init() {
        SDL_Init(SDL_INIT_VIDEO);
		renderer.init();
		audio.init(&renderer.doc);
		if (renderer.doc.HasMember("initial_scene")) {
			scene.name = renderer.doc["initial_scene"].GetString();
			scene.loadScene();
		}
		else {
			Editor::init();
			edit = true;
		}
		Rigidbody::initWorld();
		Rigidbody::physics->SetContactListener(&contact);
		return;
	}

	void collect() {
		onStart = Component::onStart;
		onUpdate = Component::onUpdate;
		onLateUpdate = Component::onLateUpdate;
	}

	void actorTrigger(string function) {
		vector<shared_ptr<luabridge::LuaRef>> iterator;
		if (function == "OnStart") {
			iterator = onStart;
			Component::onStart.clear();
		}
		else if (function == "OnUpdate") iterator = onUpdate;
		else if (function == "OnLateUpdate") iterator = onLateUpdate;
		else iterator = scene.destroyed;
		for (shared_ptr<luabridge::LuaRef> pair : iterator) {
			call(function, *pair, nullptr);
		}
	}

	static void call(string function, luabridge::LuaRef& comp, Collision* collision) {
		if (comp[function].isFunction() && (comp["enabled"] || function == "OnDestroy")) {
			try {
				if (collision == nullptr) comp[function](comp);
				else comp[function](comp, *collision);
			}
			catch (const luabridge::LuaException& e) {
				Actor* actor = (comp)["actor"];
				renderer.database.ReportError(actor->actor_name, e);
			}
		}
	}

	void luaInit() {
		luabridge::getGlobalNamespace(renderer.database.game)
			.beginNamespace("Debug")
			.addFunction("Log", &Log)
			.addFunction("LogError", &LogError)
			.endNamespace()
			.beginNamespace("Actor")
			.beginClass<Actor>("Actor")
			.addFunction("GetName", &Actor::GetName)
			.addFunction("GetID", &Actor::GetID)
			.addFunction("GetComponentByKey", &Actor::GetComponentByKey)
			.addFunction("GetComponent", &Actor::GetComponent)
			.addFunction("GetComponents", &Actor::GetComponents)
			.addFunction("AddComponent", &Actor::AddComponent)
			.addFunction("RemoveComponent", &Actor::RemoveComponent)
			.endClass()
			.addFunction("Find", &Scene::Find)
			.addFunction("FindAll", &Scene::FindAll)
			.addFunction("FindEvery", &Scene::FindEvery) //NEW
			.addFunction("Instantiate", &Scene::Instantiate)
			.addFunction("Destroy", &Scene::Destroy)
			.endNamespace()
			.beginNamespace("Application")
			.addFunction("Quit", &quit)
			.addFunction("Sleep", &sleep)
			.addFunction("GetFrame", &GetFrame)
			.addFunction("OpenURL", &OpenURL)
			.addFunction("OpenFile", &OpenFile)
			.endNamespace()
			.beginClass<glm::vec2>("vec2")
			.addProperty("x", &glm::vec2::x)
			.addProperty("y", &glm::vec2::y)
			.endClass()
			.beginNamespace("Input")
			.addFunction("GetKey", &Input::GetKeyPressed)
			.addFunction("GetKeyDown", &Input::GetKeyDown)
			.addFunction("GetKeyUp", &Input::GetKeyUp)
			.addFunction("GetMousePosition", &Input::GetMousePosition)
			.addFunction("GetMouseButton", &Input::GetMouseButtonPressed)
			.addFunction("GetMouseButtonDown", &Input::GetMouseButtonDown)
			.addFunction("GetMouseButtonUp", &Input::GetMouseButtonUp)
			.addFunction("GetMouseScrollDelta", &Input::GetMouseScrollDelta)
			.endNamespace()
			.beginNamespace("Text")
			.addFunction("Draw", &Renderer::TextDraw)
			.addFunction("Draw2", &Renderer::TextDraw2)
			.endNamespace()
			.beginNamespace("Audio")
			.addFunction("Play", &Audio::play)
			.addFunction("Halt", &AudioHelper::Mix_HaltChannel498)
			.addFunction("SetVolume", &AudioHelper::Mix_Volume498)
			.endNamespace()
			.beginNamespace("Image")
			.addFunction("DrawUI", &Renderer::DrawUI)
			.addFunction("DrawUIEx", &Renderer::DrawUIEx)
			.addFunction("Draw", &Renderer::ImageDraw)
			.addFunction("DrawEx", &Renderer::ImageDrawEx)
			.addFunction("DrawPixel", &Renderer::DrawPixel)
			.endNamespace()
			.beginNamespace("Camera")
			.addFunction("SetPosition", &Renderer::SetPosition)
			.addFunction("GetPositionX", &Renderer::GetPositionX)
			.addFunction("GetPositionY", &Renderer::GetPositionY)
			.addFunction("SetZoom", &Renderer::SetZoom)
			.addFunction("GetZoom", &Renderer::GetZoom)
			.endNamespace()
			.beginNamespace("Scene")
			.addFunction("Load", &Scene::Load)
			.addFunction("GetCurrent", &Scene::GetCurrent)
			.addFunction("DontDestroy", &Scene::DontDestroy)
			.endNamespace()
			.beginClass<b2Vec2>("Vector2")
			.addProperty("x", &b2Vec2::x)
			.addProperty("y", &b2Vec2::y)
			.addFunction("Normalize", &b2Vec2::Normalize)
			.addFunction("Length", &b2Vec2::Length)
			.addConstructor<void(*) (float, float)>()
			.addFunction("__add", &b2Vec2::operator_add)
			.addFunction("__sub", &b2Vec2::operator_sub)
			.addFunction("__mul", &b2Vec2::operator_mul)
			.endClass()
			.beginNamespace("Vector2")
			.addFunction("Distance", &b2Distance)
			.addFunction("Dot", static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Dot))
			.endNamespace()
			.beginClass<Rigidbody>("Rigidbody")
			.addData("x", &Rigidbody::x)
			.addData("y", &Rigidbody::y)
			.addData("body_type", &Rigidbody::body_type)
			.addData("precise", &Rigidbody::precise)
			.addData("gravity_scale", &Rigidbody::gravity_scale)
			.addData("density", &Rigidbody::density)
			.addData("angular_friction", &Rigidbody::angular_friction)
			.addData("rotation", &Rigidbody::rotation)
			.addData("has_collider", &Rigidbody::has_collider)
			.addData("has_trigger", &Rigidbody::has_trigger)
			.addFunction("GetPosition", &Rigidbody::GetPosition)
			.addFunction("GetRotation", &Rigidbody::GetRotation)
			.addFunction("OnStart", &Rigidbody::OnStart)
			.addFunction("OnDestroy", &Rigidbody::OnDestroy)
			.addData("key", &Rigidbody::key)
			.addData("enabled", &Rigidbody::enabled)
			.addData("nameOfTable", &Rigidbody::type)
			.addData("actor", &Rigidbody::actor)
			.addData("actorID", &Rigidbody::actorID)
			.addFunction("AddForce", &Rigidbody::AddForce)
			.addFunction("SetVelocity", &Rigidbody::SetVelocity)
			.addFunction("SetPosition", &Rigidbody::SetPosition)
			.addFunction("SetRotation", &Rigidbody::SetRotation)
			.addFunction("SetAngularVelocity", &Rigidbody::SetAngularVelocity)
			.addFunction("SetGravityScale", &Rigidbody::SetGravityScale)
			.addFunction("SetUpDirection", &Rigidbody::SetUpDirection)
			.addFunction("SetRightDirection", &Rigidbody::SetRightDirection)
			.addFunction("GetVelocity", &Rigidbody::GetVelocity)
			.addFunction("GetAngularVelocity", &Rigidbody::GetAngularVelocity)
			.addFunction("GetGravityScale", &Rigidbody::GetGravityScale)
			.addFunction("GetUpDirection", &Rigidbody::GetUpDirection)
			.addFunction("GetRightDirection", &Rigidbody::GetRightDirection)
			.addData("collider_type", &Rigidbody::collider_type)
			.addData("width", &Rigidbody::width)
			.addData("height", &Rigidbody::height)
			.addData("radius", &Rigidbody::radius)
			.addData("friction", &Rigidbody::friction)
			.addData("bounciness", &Rigidbody::bounciness)
			.addData("trigger_type", &Rigidbody::trigger_type)
			.addData("trigger_width", &Rigidbody::trigger_width)
			.addData("trigger_height", &Rigidbody::trigger_height)
			.addData("trigger_radius", &Rigidbody::trigger_radius)
			.endClass()
			.beginClass<Collision>("Collision")
			.addProperty("other", &Collision::other)
			.addProperty("point", &Collision::point)
			.addProperty("relative_velocity", &Collision::relative_velocity)
			.addProperty("normal", &Collision::normal)
			.endClass()
			.beginClass<Raycast::HitResult>("HitResult")
			.addProperty("actor", &Raycast::HitResult::actor)
			.addProperty("point", &Raycast::HitResult::point)
			.addProperty("normal", &Raycast::HitResult::normal)
			.addProperty("is_trigger", &Raycast::HitResult::is_trigger)
			.endClass()
			.beginNamespace("Physics")
			.addFunction("Raycast", &Raycast::RaycastOne)
			.addFunction("RaycastAll", &Raycast::RaycastAll)
			.endNamespace()
			.beginNamespace("Event")
			.addFunction("Publish", &Event::Publish)
			.addFunction("Subscribe", &Event::Subscribe)
			.addFunction("Unsubscribe", &Event::Unsubscribe)
			.endNamespace();
	}

	void safeQuit() {
		delete Rigidbody::physics;
		scene.reset();
		AudioHelper::Mix_CloseAudio498();
		for (SDL_Texture* current : renderer.death_note) {
			SDL_DestroyTexture(current);
		}
		SDL_DestroyRenderer(renderer.renderer);
		SDL_DestroyWindow(renderer.window);
		SDL_Quit();
		exit(0);
	}

	//LUA ENGINE API

	static void quit() {
		exit(0);
	}

	static void sleep(const int& milliseconds) {
		this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	}

	static int GetFrame() {
		return Helper::GetFrameNumber();
	}

	static void OpenURL(const string& url) {
		string command;
#ifdef _WIN32
		command = "start " + url;
#elif __APPLE__
		command = "open " + url;
#else
		command = "xdg-open " + url;
#endif
		system(command.c_str());
	}

};

int main(int argc, char* argv[]) {
	try {
		AomEngine core;
		core.run();
	}
	catch (const luabridge::LuaException& e) {
		Component::ReportError("god", e);
	}
}
