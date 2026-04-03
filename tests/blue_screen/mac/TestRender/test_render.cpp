#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("=== SDL3 macOS Test Starting ===\n");
    fflush(stdout);
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        fflush(stdout);
        return 1;
    }
    printf("SDL_Init OK!\n");
    fflush(stdout);
    
    printf("Creating window...\n");
    fflush(stdout);
    SDL_Window* window = SDL_CreateWindow("SDL3 Test", 640, 480, 0);
    
    if (!window) {
        printf("CreateWindow failed: %s\n", SDL_GetError());
        fflush(stdout);
        return 1;
    }
    printf("Window created! Showing window...\n");
    fflush(stdout);
    
    SDL_ShowWindow(window);
    printf("Window shown\n");
    fflush(stdout);
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        printf("CreateRenderer failed: %s\n", SDL_GetError());
        fflush(stdout);
        SDL_DestroyWindow(window);
        return 1;
    }
    printf("Renderer created\n");
    fflush(stdout);
    
    SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    printf("Blue screen rendered!\n");
    fflush(stdout);
    
    SDL_Event event;
    int running = 1;
    printf("Entering event loop...\n");
    fflush(stdout);
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                printf("Quit event received\n");
                fflush(stdout);
                running = 0;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                printf("Key down: %d\n", event.key.key);
                fflush(stdout);
                if (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_Q) {
                    running = 0;
                }
            }
        }
        SDL_Delay(10);
    }
    
    printf("Cleaning up...\n");
    fflush(stdout);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("Done!\n");
    fflush(stdout);
    
    return 0;
}
