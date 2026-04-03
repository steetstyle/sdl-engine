#include <SDL3/SDL.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    printf("=== SDL3 Platform Test ===\n");
    printf("SDL Version: %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("FAIL: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    printf("OK: SDL_Init succeeded\n");

    int numDrivers = SDL_GetNumVideoDrivers();
    printf("OK: Found %d video drivers\n", numDrivers);
    for (int i = 0; i < numDrivers; i++) {
        printf("  - %s\n", SDL_GetVideoDriver(i));
    }

    SDL_Window* window = SDL_CreateWindow(
        "SDL3 Render Test",
        640, 480,
        SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        printf("FAIL: SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    printf("OK: Window created (640x480)\n");

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        printf("FAIL: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    printf("OK: Renderer created\n");

    SDL_SetRenderDrawColor(renderer, 50, 100, 200, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    printf("OK: Blue screen rendered\n");

    SDL_Delay(2000);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("=== ALL TESTS PASSED ===\n");
    return 0;
}
