#include <iostream>
#include <array>
#include <vector>
#include <SDL.h>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
int PADDLE_SPEED = 16;
int BALL_SPEED = 8;

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

namespace component {
	struct Position : vec2i {};
	struct Velocity : vec2i {};
	struct Renderable : colori {
		colori color;
	};
}

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

std::ostream &operator<<(std::ostream &os, CollisionDirection dir) {
	switch (dir) {
	case CollisionDirection::Up:    return os << "Up";
	case CollisionDirection::Right: return os << "Right";
	case CollisionDirection::Down:  return os << "Down";
	case CollisionDirection::Left:  return os << "Left";
	}
	return os << static_cast<uint8_t>(dir);
}

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
		std::cout << "Collision Direction: " << collision.direction << "\n";
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
	auto collision = get_collision(ball, paddle);
	if (collision.intersects) {
		if (collision.direction == CollisionDirection::Up) ball.velocity.y = -ball.velocity.y;
	}
}

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
	ball.position = { 640, 500 };
	ball.size = { 16, 16 };
	ball.velocity = { BALL_SPEED, -BALL_SPEED };
	ball.color = colors[0];

	Paddle paddle;
	paddle.position = { 640, 680 };
	paddle.size = { 128, 16 };
	paddle.color = colors[0];

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

	std::vector<std::reference_wrapper<Entity>> renderables;
	renderables.insert(std::end(renderables), std::begin(blocks), std::end(blocks));
	renderables.emplace_back(ball);
	renderables.emplace_back(paddle);

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

		SDL_RenderPresent(renderer);
		SDL_UpdateWindowSurface(window);

		ball.position.x += ball.velocity.x;
		if (ball.position.x + ball.size.x >= WINDOW_WIDTH || ball.position.x <= 0) ball.velocity.x = -ball.velocity.x;
		handle_collisions(ball, blocks);

		ball.position.y += ball.velocity.y;
		if (ball.position.y <= 0) ball.velocity.y = -ball.velocity.y;
		if (ball.position.y > paddle.centerY()) ball.velocity = vec2i(0, 0);
		handle_collisions(ball, blocks);

		paddle.position += paddle.velocity;
		if (paddle.position.x + paddle.size.x > WINDOW_WIDTH) paddle.position.x = WINDOW_WIDTH - paddle.size.x;
		if (paddle.position.x < 0) paddle.position.x = 0;

		handle_collision(ball, paddle);

	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
