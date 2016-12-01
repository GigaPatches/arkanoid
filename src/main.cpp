#include <cassert>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <array>
#include <bitset>
#include <vector>
#include <cstdint>
#include <SDL.h>

const int GAME_WIDTH = 320;
const int GAME_HEIGHT = 180;
int PADDLE_SPEED = 4;
int BALL_SPEED = 2;

template<typename T>
struct vec2 {
	union {
		struct { T x; T y; };
		std::array<T, 2> v;
	};

	vec2() : x(0), y(0) {}
	vec2(T x, T y) : x(x), y(y) {}
	vec2(const vec2&) = default;

	vec2& normalize() {
		float len = length();
		if (len == 0) return *this;
		return *this *= 1.0 / length();
	}

	float lengthSqr() const {
		return std::sqrtf(x * x + y * y);
	}

	float length() const {
		return std::sqrtf(lengthSqr());
	}

	float dot(vec2 const& other) {
		return x * other.x + y * other.y;
	}

	float cross(vec2 const& other) {
		return x * other.y - y * other.y;
	}

	vec2& operator+=(const vec2 &rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	friend vec2 operator+(vec2 lhs, const vec2 &rhs) {
		return lhs += rhs;
	}
	vec2& operator-=(const vec2 &rhs) {
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	friend vec2 operator-(vec2 lhs, const vec2 &rhs) {
		return lhs -= rhs;
	}
	vec2& operator*=(const T &rhs) {
		x *= rhs;
		y *= rhs;
		return *this;
	}
	friend vec2 operator*(vec2 lhs, T &rhs) {
		return lhs *= rhs;
	}
	vec2& operator/=(const T &rhs) {
		x /= rhs;
		y /= rhs;
		return *this;
	}
	friend vec2 operator/(vec2 lhs, T &rhs) {
		return lhs /= rhs;
	}
	vec2 operator-() {
		vec2 v;
		v.x = -x;
		v.y = -y;
		return v;
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

	colori(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
	colori(uint32_t color) : i(color) {}
	operator uint32_t&() { return i; }
	operator uint32_t() const { return i; }
};

using vec2f = vec2<float>;
using vec2i = vec2<int>;

const std::array<std::bitset<60>, 10> FONT{
	0b111111111111'111100001111'111100001111'111100001111'111111111111, // 0
	0b000011110000'000011110000'000011110000'000011110000'000011110000, // 1
	0b111111111111'000000001111'111111111111'111100000000'111111111111, // 2
	0b111111111111'111100000000'111111111100'111100000000'111111111111, // 3
	0b111100000000'111100000000'111111111111'111100001111'111100001111, // 4
	0b111111111111'000000001111'111111111111'111100000000'111111111111, // 5
	0b111111111111'111100001111'111111111111'000000001111'111111111111, // 6
	0b111100000000'111100000000'111100000000'111100000000'111111111111, // 7
	0b111111111111'111100001111'111111111111'111100001111'111111111111, // 8
	0b111111111111'111100000000'111111111111'111100001111'111111111111, // 9
};
const int FONT_WIDTH = 12;
const int FONT_HEIGHT = 5;
const colori FONT_COLOR(142, 142, 142, 255);

struct Entity {
	union {
		struct {
			vec2i position;
			vec2i size;
		};
		SDL_Rect rect;
	};
	colori color;
	vec2i velocity;

	Entity() : position(0, 0),
		size(0, 0),
		color(0xFFFFFFFF) {}

	int centerX() const { return position.x + size.x / 2; }
	int centerY() const { return position.y + size.y / 2; }
	int left() const { return position.x; }
	int right() const { return position.x + size.x; }
	int top() const { return position.y; }
	int bottom() const { return position.y + size.y; }
};

struct Ball : Entity {};
struct Block : Entity {};
struct Paddle : Entity {};

enum class CollisionDirection : uint8_t {
	Up,
	Right,
	Down,
	Left
};

struct Collision {
	bool intersects;
	vec2i depth;
	CollisionDirection direction;
};

bool check_collision(const Entity &a, const Entity &b) {
	return a.position.x + a.size.x >= b.position.x && b.position.x + b.size.x >= a.position.x &&
		   a.position.y + a.size.y >= b.position.y && b.position.y + b.size.y >= a.position.y;
}

Collision get_collision(const Entity &a, const Entity &b) {
	Collision collision;
	if (!check_collision(a, b)) {
		collision.intersects = false;
	}
	else {
		collision.intersects = true;

		int ahw = a.size.x / 2;
		int ahh = a.size.y / 2;
		int bhw = b.size.x / 2;
		int bhh = b.size.y / 2;

		int dx = a.centerX() - b.centerX();
		int dy = a.centerY() - b.centerY();
		int mx = ahw + bhw;
		int my = ahh + bhh;
		collision.depth = vec2i(mx - dx, my - dy);


		if (dy < 0) collision.direction = CollisionDirection::Up;
		else if (dy > 0) collision.direction = CollisionDirection::Down;
		else if (dx < 0) collision.direction = CollisionDirection::Left;
		else if (dx > 0) collision.direction = CollisionDirection::Right;
	}

	return collision;
}

void handle_collisions(Ball &ball, std::vector<Block> &blocks) {
	vec2i new_velocity(ball.velocity);
	for (auto &&brick : blocks) {
		if (brick.color.a) {
			auto collision = get_collision(ball, brick);
			if (collision.intersects) {
				auto &velocity = ball.velocity;

				brick.color.a = 0;
				switch (collision.direction) {
				case CollisionDirection::Up:
				case CollisionDirection::Down:
					ball.position.y -= collision.depth.y;
					new_velocity.y = -velocity.y;
					break;
				case CollisionDirection::Right:
				case CollisionDirection::Left:
					ball.position.x -= collision.depth.x;
					new_velocity.x = -velocity.x;
					break;
				}
			}
		}
	}
	ball.velocity = new_velocity;
}

void handle_collision(Ball &ball, Paddle &paddle) {
	static bool was_colliding = false;
	auto collision = get_collision(ball, paddle);
	if (collision.intersects && !was_colliding) {
		ball.velocity.y = -ball.velocity.y;
		if (collision.direction == CollisionDirection::Down) ball.velocity = vec2i(0, 0);

	}
	was_colliding = collision.intersects;
}			

SDL_Texture *create_font_texture(SDL_Renderer *renderer) {
	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, FONT_WIDTH * FONT.size(), FONT_HEIGHT);
	std::vector<uint32_t> pixel_data;
	pixel_data.reserve(FONT_WIDTH * FONT.size() * FONT_HEIGHT);
	for (int y = 0; y < FONT_HEIGHT; ++y) {
		for (auto &glyph : FONT) {
			for (int x = 0; x < FONT_WIDTH; ++x) {
				pixel_data.push_back(0xFFFFFFFF * glyph[x + y * FONT_WIDTH]);
			}
		}
	}

	int stride = 4 * FONT_WIDTH * FONT.size();

	assert(SDL_UpdateTexture(texture, nullptr, pixel_data.data(), stride) == 0);
	return texture;
}

void draw(SDL_Renderer *renderer, SDL_Texture *texture, vec2i pos, int num, int width = 0) {
	static std::array<uint8_t, 10> digits;
	SDL_Rect source{ 0,0,FONT_WIDTH,FONT_HEIGHT };
	SDL_Rect dest{ 0,pos.y,FONT_WIDTH,FONT_HEIGHT };

	int len = 0;
	while (num != 0) {
		digits[len++] = num % 10;
		num /= 10;
	}
	
	while (len < width) {
		digits[len++] = 0;
	}
	
	SDL_SetTextureColorMod(texture, FONT_COLOR.r, FONT_COLOR.g, FONT_COLOR.b);
	for (int i = len - 1; i >= 0; --i) {
		source.x = digits[i] * FONT_WIDTH;
		SDL_RenderCopy(renderer, texture, &source, &dest);
		dest.x += FONT_WIDTH + 4;
	}
}

int main(int, char**) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
		return 1;
	}

	auto window = SDL_CreateWindow("Arkanoid", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, 0);
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
	auto font_texture = create_font_texture(renderer);

	const std::array<colori, 6> colors{
		colori(200,72,72,255),
		colori(198,108,58,255),
		colori(180,122,48,255),
		colori(162,162,42,255),
		colori(72,160,72,255),
		colori(66,72,200,255)
	};

	Ball ball;
	ball.position = { 160, 125 };
	ball.size = { 4, 4 };
	ball.velocity = { BALL_SPEED, -BALL_SPEED };
	ball.color = colors[0];

	Paddle paddle;
	paddle.position = { 160, 170 };
	paddle.size = { 32, 4 };
	paddle.color = colors[0];

	std::vector<Block> blocks;

	for (int y = 0; y < 12; y++) {
		for (int x = 0; x < 10; x++) {
			Block block;
			block.size = { 32, 4 };
			block.position = { x*block.size.x, y*block.size.y };
			block.color = colors[y/2];
			blocks.push_back(block);
		}
	}

	std::vector<std::reference_wrapper<Entity>> renderables;
	renderables.insert(std::end(renderables), std::begin(blocks), std::end(blocks));
	renderables.emplace_back(ball);
	renderables.emplace_back(paddle);

	SDL_RenderSetLogicalSize(renderer, GAME_WIDTH, GAME_HEIGHT);
	SDL_Event e;
	bool quit = false;
	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
			else if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case SDLK_LEFT:
				case SDLK_a:
					paddle.velocity.x = -PADDLE_SPEED;
					break;
				case SDLK_RIGHT:
				case SDLK_d:
					paddle.velocity.x = PADDLE_SPEED;
					break;
				}
			}
			else if (e.type == SDL_KEYUP) {
				switch (e.key.keysym.sym) {
				case SDLK_LEFT:
				case SDLK_a:
					if (paddle.velocity.x < 0)
						paddle.velocity.x = 0;
					break;
				case SDLK_RIGHT:
				case SDLK_d:
					if (paddle.velocity.x > 0)
						paddle.velocity.x = 0;
					break;
				}
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		for (auto& render : renderables) {
			auto obj = render.get();
			if (!obj.color.a) continue;
			SDL_SetRenderDrawColor(renderer, obj.color.r, obj.color.g, obj.color.b, obj.color.a);
			SDL_RenderFillRect(renderer, &obj.rect);
		}

		draw(renderer, font_texture, vec2i(0, 100), 1234567890);

		SDL_RenderPresent(renderer);
		SDL_UpdateWindowSurface(window);

		ball.position.x += ball.velocity.x;
		if (ball.position.x + ball.size.x >= GAME_WIDTH || ball.position.x <= 0) ball.velocity.x = -ball.velocity.x;
		handle_collisions(ball, blocks);

		ball.position.y += ball.velocity.y;
		if (ball.position.y <= 0) ball.velocity.y = -ball.velocity.y;
		if (ball.position.y >= GAME_HEIGHT - ball.size.y) ball.velocity = vec2i(0, 0);
		handle_collisions(ball, blocks);

		paddle.position += paddle.velocity;
		if (paddle.position.x + paddle.size.x > GAME_WIDTH) paddle.position.x = GAME_WIDTH - paddle.size.x;
		if (paddle.position.x < 0) paddle.position.x = 0;

		handle_collision(ball, paddle);
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
