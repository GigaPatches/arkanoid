#include <iostream>
#include <SDL.h>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

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

    SDL_Rect rect = { 0, 0, 32, 32 };
    int xvel = 1;
    int yvel = 1;

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

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &rect);

        SDL_RenderPresent(renderer);
        SDL_UpdateWindowSurface(window);

        rect.x += xvel;
        rect.y += yvel;
        if (rect.x + 32 >= WINDOW_WIDTH || rect.x <= 0) xvel = -xvel;
        if (rect.y + 32 >= WINDOW_HEIGHT || rect.y <= 0) yvel = -yvel;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
