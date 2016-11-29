#include <iostream>
#include <array>
#include <vector>
#include <SDL.h>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

template<typename T>
struct vec2 {
	union {
		struct { T x; T y; };
		T v[2];
	};

	vec2() : x(0), y(0) {}
	vec2(T x, T y) : x(x), y(y) {}
	vec2(const vec2&) = default;

	vec2<T>& operator+=(const vec2<T> &rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	friend vec2<T> operator+(vec2<T> lhs, const vec2<T> &rhs) {
		return lhs += rhs;
	}
	vec2<T>& operator-=(const vec2<T> &rhs) {
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	friend vec2<T> operator-(vec2<T> lhs, const vec2<T> &rhs) {
		return lhs -= rhs;
	}
	vec2<T>& operator*=(const T &rhs) {
		x *= rhs;
		y *= rhs;
		return *this;
	}
	friend vec2<T> operator*(vec2<T> lhs, T &rhs) {
		return lhs *= rhs;
	}
	vec2<T>& operator/=(const T &rhs) {
		x /= rhs;
		y /= rhs;
		return *this;
	}
	friend vec2<T> operator/(vec2<T> lhs, T &rhs) {
		return lhs /= rhs;
	}
};
struct colori {
	union {
		struct {
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		};
		uint32_t i;
	};

	colori(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a){}
};

using vec2f = vec2<float>;
using vec2i = vec2<int>;

struct Renderable {
	union {
		struct {
			vec2i position;
			vec2i size;
		};
		SDL_Rect rect;
	};
	colori color;

	Renderable() :
		position(0, 0), 
		size(0, 0),
		color({ 255,255,255,255 }) {}
};

struct Ball : Renderable {
	vec2i velocity;

	Ball() : velocity(0, 0) {}
};

struct Block : Renderable {
};

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return 1;
    }

    auto window = SDL_CreateWindow("Arkanoid", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (window == nullptr) {
        std::cerr << "Window creation failed! SDL_Error: " << SDL_GetError() << "\n";
        return 2;
    }

    auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 3;
    }
	

	const std::array<colori, 6> colors{
		colori(200,72,72,255),
		colori(198,108,58,255),
		colori(180,122,48,255),
		colori(162,162,42,255),
		colori(72,160,72,255),
		colori(66,72,200,255)
	};

	Ball ball;
	ball.size = { 16, 16 };
	ball.velocity = { 4, 8 };
	ball.color = colors[0];

	std::vector<Block> blocks;
	
	for (int y = 0; y < 6; y++) {
		for (int x = 0; x < 10; x++) {
			Block block;
			block.size = { 128, 32 };
			block.position = { x*block.size.x, y*block.size.y };
			block.color = colors[y];
			blocks.push_back(block);
		}
	}

	std::vector<std::reference_wrapper<Renderable>> renderables;
	renderables.insert(std::end(renderables), std::begin(blocks), std::end(blocks));
	renderables.push_back(ball);

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

		for (auto& render : renderables) {
			auto obj = render.get();
			SDL_SetRenderDrawColor(renderer, obj.color.r, obj.color.g, obj.color.b, obj.color.a);
			SDL_RenderFillRect(renderer, &obj.rect);
		}

        SDL_RenderPresent(renderer);
        SDL_UpdateWindowSurface(window);

		ball.position += ball.velocity;
        if (ball.position.x + ball.size.x >= WINDOW_WIDTH || ball.position.x <= 0) ball.velocity.x = -ball.velocity.x;
        if (ball.position.y + ball.size.y >= WINDOW_HEIGHT || ball.position.y <= 0) ball.velocity.y = -ball.velocity.y;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
