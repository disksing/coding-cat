#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <windows.h>
#include "SDL.h"
#include "SDL_image.h"
using namespace std;

class Images {
public:
	void load_background() {
		_background = IMG_Load("assets/background.png");
		if (_background == nullptr) {
			exit(-1);
		}
		_width = _background->w;
		_height = _background->h;
	}

	void create_textures(SDL_Renderer* renderer) {
		_renderer = renderer;

		_textures["background"] = SDL_CreateTextureFromSurface(renderer, _background);
		SDL_FreeSurface(_background);
		_background = nullptr;

		char name[16], file[64];
		for (int i = 0; i <= 3; i++) {
			sprintf_s(name, "left%d", i);
			sprintf_s(file, "assets/%s.png", name);
			auto s = IMG_Load(file);
			_textures[string(name)] = SDL_CreateTextureFromSurface(renderer, s);
			SDL_FreeSurface(s);
		}
		for (int i = 0; i <= 5; i++) {
			sprintf_s(name, "right%d", i);
			sprintf_s(file, "assets/%s.png", name);
			auto s = IMG_Load(file);
			_textures[string(name)] = SDL_CreateTextureFromSurface(renderer, s);
			SDL_FreeSurface(s);
		}
	}

	pair<int,int> get_window_size() {
		return make_pair(_width, _height);
	}

	void render_background() {
		SDL_RenderCopy(_renderer, _textures["background"], nullptr, nullptr);
	}

	void render_left(int i) {
		string s("left");
		s.push_back('0' + i);
		SDL_RenderCopy(_renderer, _textures[s], nullptr, nullptr);
	}

	void render_right(int i) {
		string s("right");
		s.push_back('0' + i);
		SDL_RenderCopy(_renderer, _textures[s], nullptr, nullptr);
	}

private:
	SDL_Surface* _background;
	int _width, _height;
	SDL_Renderer* _renderer;
	map<string, SDL_Texture*> _textures;
};

static const int FPS = 60;
static const int FrameInterval = 1000 / FPS;
static const int CatResetDelay = 1000;

static const vector<int> Left2Keys = {VK_ESCAPE, '1', '2', '3', VK_TAB, 'Q', 'W', 'E', VK_CAPITAL, 'A', 'S', VK_LSHIFT, 'Z', 'X', VK_LCONTROL, VK_LWIN, VK_LMENU};
static const vector<int> Left3Keys = {'4', '5', '6', 'R', 'T', 'D', 'F', 'G', 'C', 'V', 'B', VK_SPACE};
static const vector<int> Right2Keys = {'7', '8', '9', 'Y', 'U', 'I', 'O', 'H', 'J', 'K','N', 'M', VK_OEM_COMMA, VK_RMENU};
static const vector<int> Right3Keys = {'0', VK_OEM_MINUS, VK_OEM_PLUS, VK_BACK, 'P', VK_OEM_4, VK_OEM_6, VK_OEM_5, 'L', VK_OEM_1, VK_OEM_7, VK_RETURN, VK_OEM_PERIOD, VK_OEM_2, VK_RSHIFT, VK_RCONTROL, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT};
static const vector<int> MouseKeys = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON };

class Scene {
public:
	void render(SDL_Renderer* renderer, Images *images) {
		SDL_RenderClear(renderer);
		images->render_background();
		images->render_left(_left_index);
		images->render_right(_right_index);
		SDL_RenderPresent(renderer);
	}

	void update(UINT32 tick) {
		_update_states();

		if (_next_left > 0) {
			_left_index = _next_left;
			_left_reset_tick = 0;
		} else if (_left_index == 2 || _left_index == 3) {
			_left_index = 1;
			_left_reset_tick = tick + CatResetDelay;
		}

		if (_next_right > 0) {
			_right_index = _next_right;
			_right_reset_tick = 0;
			if (_next_right == 4) {
				_right_reset_tick = tick + CatResetDelay;
			}
		} else if (_right_index == 2 || _right_index == 3) {
			_right_index = 1;
			_right_reset_tick = tick + CatResetDelay;
		} else if (_right_index == 5) {
			_right_index = 4;
			_right_reset_tick = tick + CatResetDelay;
		}

		if (_left_reset_tick > 0 && tick >= _left_reset_tick) {
			_left_index = 0;
			_left_reset_tick = 0;
		}
		if (_right_reset_tick > 0 && tick >= _right_reset_tick) {
			_right_index = 0;
			_right_reset_tick = 0;
		}
	}

private:
	bool _any_key_pressed(const vector<int>& keys) {
		for (auto it = keys.begin(); it != keys.end(); it++) {
			if (GetKeyState(*it) & 0x8000) {
				return true;
			}
		}
		return false;
	}

	bool _mouse_moved() {
		POINT pt;
		GetCursorPos(&pt);
		bool changed = (pt.x != _cursor_pos.x || pt.y != _cursor_pos.y);
		_cursor_pos = pt;
		return changed;
	}

	void _update_states() {
		_next_left = 0;
		if (_any_key_pressed(Left2Keys)) {
			_next_left = 2;
		}
		if (_any_key_pressed(Left3Keys)) {
			_next_left = 3;
		}
		_next_right = 0;
		if (_any_key_pressed(Right2Keys)) {
			_next_right = 2;
		}
		if (_any_key_pressed(Right3Keys)) {
			_next_right = 3;
		}
		if (_mouse_moved()) {
			_next_right = 4;
		}
		if (_any_key_pressed(MouseKeys)) {
			_next_right = 5;
		}
	}

	int _left_index = 0;
	UINT32 _left_reset_tick = 0;
	int _right_index = 0;
	UINT32 _right_reset_tick = 0;

	int _next_left = 0;
	int _next_right = 0;
	POINT _cursor_pos = { 0,0 };
};

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_EVERYTHING);
	
	Images images;
	images.load_background();
	auto window_size = images.get_window_size();

	SDL_Window* window = SDL_CreateWindow("Coding Cat",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		window_size.first, window_size.second, SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	images.create_textures(renderer);

	Scene scene;
	while (true) {
		auto tick = SDL_GetTicks();
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				SDL_DestroyWindow(window);
				SDL_DestroyRenderer(renderer);
				SDL_Quit();
				return 0;
			}
		}
		scene.update(tick);
		scene.render(renderer, &images);
		auto elapsed = SDL_GetTicks() - tick;
		if (elapsed < FrameInterval) {
			SDL_Delay(FrameInterval - elapsed);
		}
	}
}