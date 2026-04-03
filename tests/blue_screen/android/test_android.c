#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
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

    printf("=== ALL TESTS PASSED ===\n");
    SDL_Quit();
    return 0;
}
