#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <stdio.h>

static SDL_Window* g_window = NULL;
static SDL_Renderer* g_renderer = NULL;

extern "C" void MyCallback(void* param) {
    (void)param;
    
    SDL_SetRenderDrawColor(g_renderer, 0, 100, 255, 255);
    SDL_RenderClear(g_renderer);
    SDL_RenderPresent(g_renderer);
}

int main(int argc, char *argv[]) {
    printf("=== SDL3 Test Starting ===\n");
    fflush(stdout);
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        fflush(stdout);
        return 1;
    }
    printf("SDL_Init OK!\n");
    fflush(stdout);
    
    g_window = SDL_CreateWindow("SDL3 Test", 480, 320, 0);
    
    if (!g_window) {
        printf("CreateWindow failed: %s\n", SDL_GetError());
        fflush(stdout);
        return 1;
    }
    printf("Window created\n");
    fflush(stdout);
    
    g_renderer = SDL_CreateRenderer(g_window, NULL);
    if (!g_renderer) {
        printf("CreateRenderer failed: %s\n", SDL_GetError());
        fflush(stdout);
        SDL_DestroyWindow(g_window);
        return 1;
    }
    printf("Renderer created\n");
    fflush(stdout);
    
    printf("Setting up animation callback...\n");
    fflush(stdout);
    
    SDL_SetiOSAnimationCallback(g_window, 1, MyCallback, NULL);
    printf("Animation callback set! Running for 10 seconds...\n");
    fflush(stdout);
    
    SDL_Delay(10000);
    
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
    printf("Done!\n");
    fflush(stdout);
    
    return 0;
}