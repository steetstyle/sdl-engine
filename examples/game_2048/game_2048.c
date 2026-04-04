#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define GRID_SIZE 4
#define CELL_COUNT (GRID_SIZE * GRID_SIZE)
static int HEADER_HEIGHT = 80;
static int gridOffsetX = 0;
static int gridOffsetY = 0;
#define ANIM_SPEED 0.9f

static int TILE_SIZE = 100;
static int GAP = 10;
static int WINDOW_SIZE = 480;

typedef struct {
    int value;
    float x, y;
    float targetX, targetY;
    float scale;
    float targetScale;
    float destScale;
    int merged;
    int newlySpawned;
    int removeAnim;
    float alpha;
    int prevValue;
    float prevX, prevY;
} Tile;

typedef struct {
    Tile tiles[CELL_COUNT];
    int tileCount;
    int score;
    int displayScore;
    int scoreGained;
    int bestScore;
    int gameOver;
    int won;
    float gameOverAlpha;
    int inputBlocked;
} Game;

static Uint32 tileBgColors[18] = {
    0xFFCDC1B4, 0xFFEEE4DA, 0xFFEDE0C8, 0xFFF2B179,
    0xFFF59563, 0xFFF67C5F, 0xFFF65E3B, 0xFFEDCF72,
    0xFFEDCC61, 0xFFEDC850, 0xFFEDC53F, 0xFFEDC22E,
    0xFF8F7A6E, 0xFF8F7A6E, 0xFF8F7A6E, 0xFF8F7A6E,
    0xFF8F7A6E, 0xFF8F7A6E
};

static SDL_Color textDark = {119, 110, 101, 255};
static SDL_Color textLight = {249, 245, 235, 255};
static SDL_Color textGold = {255, 215, 0, 255};
static SDL_Color bgLight = {187, 173, 160, 255};
static SDL_Color cellEmpty = {205, 193, 180, 255};

static TTF_Font* font = NULL;
static SDL_Texture* numberTextures[18] = {NULL};
static int numberValues[18] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144};

static SDL_Texture* scoreTex = NULL;
static SDL_Texture* bestTex = NULL;
static SDL_Texture* restartTex = NULL;
static SDL_Texture* plusTex = NULL;
static SDL_Texture* gameOverTex = NULL;
static SDL_Texture* tapRestartTex = NULL;
static SDL_Texture* gridBgTex = NULL;

typedef struct {
    int score;
    int bestScore;
    int tiles[CELL_COUNT];
} SaveData;

static int getNumberFontSize(int value);
static const char* getSavePath();

static void saveGame(Game* game) {
    SDL_Log("Saving game state...");
    const char* path = getSavePath();
    FILE* f = fopen(path, "wb");
    if (!f) {
        SDL_Log("Failed to open save file: %s", path);
        return;
    }
    SaveData saveData;
    saveData.score = game->score;
    saveData.bestScore = game->bestScore;
    for (int i = 0; i < CELL_COUNT; i++) {
        saveData.tiles[i] = game->tiles[i].value;
    }
    fwrite(&saveData, sizeof(SaveData), 1, f);
    fclose(f);
    SDL_Log("Game saved to %s", path);
}

static int loadGame(Game* game) {
    const char* path = getSavePath();
    FILE* f = fopen(path, "rb");
    if (!f) {
        SDL_Log("No save file found, starting new game");
        return 0;
    }
    SaveData saved;
    size_t read = fread(&saved, sizeof(SaveData), 1, f);
    fclose(f);
    if (read != 1) {
        SDL_Log("Failed to read save file");
        return 0;
    }
    memset(game, 0, sizeof(Game));
    game->score = saved.score;
    game->bestScore = saved.bestScore;
    game->displayScore = saved.score;
    for (int i = 0; i < CELL_COUNT; i++) {
        game->tiles[i].value = saved.tiles[i];
        game->tiles[i].x = (float)(i % GRID_SIZE);
        game->tiles[i].y = (float)(i / GRID_SIZE);
        game->tiles[i].targetX = game->tiles[i].x;
        game->tiles[i].targetY = game->tiles[i].y;
        game->tiles[i].scale = game->tiles[i].value > 0 ? 1.0f : 0.0f;
        game->tiles[i].targetScale = game->tiles[i].scale;
        game->tiles[i].destScale = game->tiles[i].scale;
    }
    game->inputBlocked = 0;
    game->gameOver = 0;
    game->gameOverAlpha = 0;
    SDL_Log("Game loaded from %s", path);
    return 1;
}

static void initNumberTextures(SDL_Renderer* renderer) {
    for (int i = 0; i < 18; i++) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", numberValues[i]);
        size_t len = strlen(buf);
        
        SDL_Color tc = (i < 2) ? textDark : textLight;
        int fontSize = getNumberFontSize(numberValues[i]);
        TTF_SetFontSize(font, fontSize);
        SDL_Surface* surf = TTF_RenderText_Blended(font, buf, len, tc);
        if (!surf) continue;
        
        numberTextures[i] = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_DestroySurface(surf);
    }
}

static void cleanupNumberTextures() {
    for (int i = 0; i < 18; i++) {
        if (numberTextures[i]) {
            SDL_DestroyTexture(numberTextures[i]);
            numberTextures[i] = NULL;
        }
    }
    if (scoreTex) { SDL_DestroyTexture(scoreTex); scoreTex = NULL; }
    if (bestTex) { SDL_DestroyTexture(bestTex); bestTex = NULL; }
    if (restartTex) { SDL_DestroyTexture(restartTex); restartTex = NULL; }
    if (plusTex) { SDL_DestroyTexture(plusTex); plusTex = NULL; }
    if (gameOverTex) { SDL_DestroyTexture(gameOverTex); gameOverTex = NULL; }
    if (tapRestartTex) { SDL_DestroyTexture(tapRestartTex); tapRestartTex = NULL; }
}

static SDL_Texture* getNumberTexture(int value) {
    int log2 = 0;
    int v = value;
    while (v > 1) { v >>= 1; log2++; }
    if (log2 >= 17) log2 = 17;
    if (log2 < 0) return NULL;
    return numberTextures[log2];
}

static int getNumberFontSize(int value) {
    if (value >= 1000000) return 28;
    if (value >= 100000) return 32;
    if (value >= 10000) return 38;
    if (value >= 1000) return 44;
    if (value >= 100) return 52;
    if (value >= 10) return 60;
    return 70;
}

static const char* getFontPath() {
#ifdef __ANDROID__
    SDL_Log("Android platform detected, loading font from assets");
    return "DejaVuSans-Bold.ttf";
#else
    const char* path = "/home/roy/github-projects/sdl-engine/examples/android/app/src/main/assets/DejaVuSans-Bold.ttf";
    SDL_Log("Linux platform, loading font from: %s", path);
    return path;
#endif
}

static const char* getSavePath() {
#ifdef __ANDROID__
    return "/data/data/org.libsdl.app/files/savegame.dat";
#else
    return "/home/roy/github-projects/sdl-engine/examples/game_2048/savegame.dat";
#endif
}

static void spawnTile(Game* game);
static void initGame(Game* game);
static int moveTiles(Game* game, int dx, int dy);
static void drawTile(SDL_Renderer* renderer, Tile* tile);
static void drawGame(SDL_Renderer* renderer, Game* game);
static int checkGameOver(Game* game);
static void updateAnimations(Game* game);
static int isAnimating(Game* game);

static int findEmptyCell(Game* game) {
    int emptyIndices[CELL_COUNT];
    int count = 0;
    for (int i = 0; i < CELL_COUNT; i++) {
        if (game->tiles[i].value == 0 && game->tiles[i].scale <= 0.01f && !game->tiles[i].removeAnim) {
            emptyIndices[count++] = i;
        }
    }
    if (count == 0) return -1;
    return emptyIndices[rand() % count];
}

static void spawnTile(Game* game) {
    int idx = findEmptyCell(game);
    if (idx == -1) return;
    game->tiles[idx].value = (rand() % 10 < 9) ? 2 : 4;
    game->tiles[idx].x = (float)(idx % GRID_SIZE);
    game->tiles[idx].y = (float)(idx / GRID_SIZE);
    game->tiles[idx].targetX = game->tiles[idx].x;
    game->tiles[idx].targetY = game->tiles[idx].y;
    game->tiles[idx].scale = 0.0f;
    game->tiles[idx].targetScale = 1.0f;
    game->tiles[idx].destScale = 1.0f;
    game->tiles[idx].newlySpawned = 1;
    game->tiles[idx].alpha = 1.0f;
    game->tiles[idx].merged = 0;
    game->tiles[idx].removeAnim = 0;
    game->tileCount++;
}

static void initGame(Game* game) {
    memset(game, 0, sizeof(Game));
    game->score = 0;
    game->displayScore = 0;
    game->scoreGained = 0;
    srand((unsigned int)time(NULL));
    for (int i = 0; i < CELL_COUNT; i++) {
        game->tiles[i].value = 0;
        game->tiles[i].x = (float)(i % GRID_SIZE);
        game->tiles[i].y = (float)(i / GRID_SIZE);
        game->tiles[i].targetX = game->tiles[i].x;
        game->tiles[i].targetY = game->tiles[i].y;
        game->tiles[i].scale = 0.0f;
        game->tiles[i].targetScale = 0.0f;
        game->tiles[i].destScale = 0.0f;
        game->tiles[i].newlySpawned = 0;
        game->tiles[i].merged = 0;
        game->tiles[i].removeAnim = 0;
        game->tiles[i].alpha = 0.0f;
    }
    spawnTile(game);
    spawnTile(game);
    game->inputBlocked = 0;
}

static int moveTiles(Game* game, int dx, int dy) {
    if (game->inputBlocked) return 0;

    int moved = 0;
    int next_values[GRID_SIZE][GRID_SIZE];
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            next_values[y][x] = game->tiles[y * GRID_SIZE + x].value;
        }
    }

    for (int i = 0; i < GRID_SIZE; i++) {
        int line[GRID_SIZE] = {0};
        int count = 0;

        for (int j = 0; j < GRID_SIZE; j++) {
            int x = (dx != 0) ? j : i;
            int y = (dx != 0) ? i : j;
            if (next_values[y][x] != 0) {
                line[count++] = next_values[y][x];
            }
        }

        if (dx > 0 || dy > 0) {
            for (int low = 0, high = count - 1; low < high; low++, high--) {
                int tmp = line[low]; line[low] = line[high]; line[high] = tmp;
            }
        }

        int newLine[GRID_SIZE] = {0};
        int mergedLine[GRID_SIZE] = {0};
        int newCount = 0;
        for (int j = 0; j < count; j++) {
            if (j + 1 < count && line[j] == line[j + 1] && line[j] < 2048) {
                newLine[newCount] = line[j] * 2;
                mergedLine[newCount] = 1;
                game->score += newLine[newCount];
                game->scoreGained = newLine[newCount];
                j++;
                newCount++;
            } else {
                newLine[newCount] = line[j];
                mergedLine[newCount] = 0;
                newCount++;
            }
        }

        while (newCount < GRID_SIZE) {
            newLine[newCount] = 0;
            mergedLine[newCount] = 0;
            newCount++;
        }

        if (dx > 0 || dy > 0) {
            for (int low = 0, high = GRID_SIZE - 1; low < high; low++, high--) {
                int tmp = newLine[low]; newLine[low] = newLine[high]; newLine[high] = tmp;
                int tmp2 = mergedLine[low]; mergedLine[low] = mergedLine[high]; mergedLine[high] = tmp2;
            }
        }

        for (int j = 0; j < GRID_SIZE; j++) {
            int x = (dx != 0) ? j : i;
            int y = (dx != 0) ? i : j;
            int idx = y * GRID_SIZE + x;
            
            int oldValue = game->tiles[idx].value;
            int newValue = newLine[j];
            int isMerged = mergedLine[j];
            
            if (oldValue != newValue) {
                moved = 1;
            }
            
            if (oldValue != 0) {
                game->tiles[idx].x = (float)(oldValue != 0 ? (idx % GRID_SIZE) : x);
                game->tiles[idx].y = (float)(oldValue != 0 ? (idx / GRID_SIZE) : y);
            }
            
            game->tiles[idx].value = newValue;
            
            if (newValue > 0) {
                game->tiles[idx].x = (float)x;
                game->tiles[idx].y = (float)y;
                game->tiles[idx].targetX = (float)x;
                game->tiles[idx].targetY = (float)y;
                game->tiles[idx].scale = isMerged ? 1.2f : 1.0f;
                game->tiles[idx].targetScale = 1.0f;
                game->tiles[idx].destScale = 1.0f;
                game->tiles[idx].merged = isMerged;
                game->tiles[idx].newlySpawned = 0;
                game->tiles[idx].removeAnim = 0;
            } else {
                game->tiles[idx].scale = 0.0f;
                game->tiles[idx].targetScale = 0.0f;
                game->tiles[idx].destScale = 0.0f;
            }
        }
    }

    if (moved) {
        game->inputBlocked = 1;
    }
    return moved;
}

static int checkGameOver(Game* game) {
    for (int i = 0; i < CELL_COUNT; i++) {
        if (game->tiles[i].value == 0 && game->tiles[i].scale <= 0.01f) return 0;
    }

    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            int idx = y * GRID_SIZE + x;
            int val = game->tiles[idx].value;
            if (val == 0) continue;
            if (x < GRID_SIZE - 1 && game->tiles[idx + 1].value == val) return 0;
            if (y < GRID_SIZE - 1 && game->tiles[idx + GRID_SIZE].value == val) return 0;
        }
    }

    return 1;
}

static void updateAnimations(Game* game) {
    game->inputBlocked = 0;

    for (int i = 0; i < CELL_COUNT; i++) {
        game->tiles[i].x = game->tiles[i].targetX;
        game->tiles[i].y = game->tiles[i].targetY;

        if (game->tiles[i].removeAnim) {
            game->tiles[i].scale = 0;
            game->tiles[i].removeAnim = 0;
            game->tiles[i].value = 0;
            continue;
        }

        game->tiles[i].scale = game->tiles[i].targetScale;
        game->tiles[i].merged = 0;
        game->tiles[i].newlySpawned = 0;
    }

    game->displayScore = game->score;
    game->scoreGained = 0;
}

static void renderText(SDL_Renderer* r, TTF_Font* f, const char* text, int x, int y, SDL_Color color, int align) {
    size_t len = strlen(text);
    SDL_Surface* surf = TTF_RenderText_Blended(f, text, len, color);
    if (!surf) return;
    
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    if (!tex) {
        SDL_DestroySurface(surf);
        return;
    }
    
    int w = surf->w;
    int h = surf->h;
    SDL_DestroySurface(surf);
    
    if (align == 1) x -= w / 2;
    else if (align == 2) x -= w;
    
    SDL_FRect dst = {(float)x, (float)y, (float)w, (float)h};
    SDL_RenderTexture(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

static void drawTile(SDL_Renderer* renderer, Tile* tile) {
    if (tile->value == 0 && tile->scale <= 0.01f) return;

    int value = tile->value;
    if (value == 0 && tile->prevValue > 0) {
        value = tile->prevValue;
    }
    if (value == 0) return;
    
    int log2 = 0;
    int v = value;
    while (v > 1) { v >>= 1; log2++; }
    if (log2 >= 17) log2 = 17;

    int baseX = gridOffsetX + GAP + (int)(tile->x * (TILE_SIZE + GAP));
    int baseY = gridOffsetY + GAP + (int)(tile->y * (TILE_SIZE + GAP));
    
    int size = (int)((float)TILE_SIZE * tile->scale);
    int offset = (TILE_SIZE - size) / 2;
    int x = baseX + offset;
    int y = baseY + offset;

    if (size <= 0) return;

    SDL_FRect rect = {(float)x, (float)y, (float)size, (float)size};
    Uint32 color = tileBgColors[log2];
    SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 255);
    SDL_RenderFillRect(renderer, &rect);

    SDL_Texture* tex = getNumberTexture(value);
    if (tex && tile->scale > 0.3f) {
        float texW, texH;
        SDL_GetTextureSize(tex, &texW, &texH);
        float scale = tile->scale * (float)size / (float)TILE_SIZE;
        int drawW = (int)(texW * scale);
        int drawH = (int)(texH * scale);
        SDL_FRect dst = {(float)(x + size/2 - drawW/2), (float)(y + size/2 - drawH/2), (float)drawW, (float)drawH};
        SDL_RenderTexture(renderer, tex, NULL, &dst);
    }
}

static int lastScore = -1;
static int lastBestScore = -1;
static int lastScoreGained = -1;

static void updateScoreTextures(SDL_Renderer* renderer, Game* game) {
    if (game->displayScore != lastScore) {
        if (scoreTex) SDL_DestroyTexture(scoreTex);
        char buf[32]; snprintf(buf, sizeof(buf), "Score: %d", game->displayScore);
        TTF_SetFontSize(font, 20);
        SDL_Surface* surf = TTF_RenderText_Blended(font, buf, strlen(buf), textDark);
        if (surf) { scoreTex = SDL_CreateTextureFromSurface(renderer, surf); SDL_DestroySurface(surf); }
        lastScore = game->displayScore;
    }
    if (game->bestScore != lastBestScore) {
        if (bestTex) SDL_DestroyTexture(bestTex);
        char buf[32]; snprintf(buf, sizeof(buf), "Best: %d", game->bestScore);
        TTF_SetFontSize(font, 20);
        SDL_Surface* surf = TTF_RenderText_Blended(font, buf, strlen(buf), textDark);
        if (surf) { bestTex = SDL_CreateTextureFromSurface(renderer, surf); SDL_DestroySurface(surf); }
        lastBestScore = game->bestScore;
    }
    if (game->scoreGained != lastScoreGained) {
        if (plusTex) SDL_DestroyTexture(plusTex);
        if (game->scoreGained > 0) {
            char buf[16]; snprintf(buf, sizeof(buf), "+%d", game->scoreGained);
            TTF_SetFontSize(font, 18);
            SDL_Surface* surf = TTF_RenderText_Blended(font, buf, strlen(buf), (SDL_Color){100, 200, 100, 255});
            if (surf) { plusTex = SDL_CreateTextureFromSurface(renderer, surf); SDL_DestroySurface(surf); }
        }
        lastScoreGained = game->scoreGained;
    }
}

static void createGridBg(SDL_Renderer* renderer) {
    if (gridBgTex) SDL_DestroyTexture(gridBgTex);
    gridBgTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, TILE_SIZE * GRID_SIZE + GAP * (GRID_SIZE + 1), TILE_SIZE * GRID_SIZE + GAP * (GRID_SIZE + 1));
    SDL_SetTextureBlendMode(gridBgTex, SDL_BLENDMODE_NONE);
    SDL_SetRenderTarget(renderer, gridBgTex);
    SDL_SetRenderDrawColor(renderer, cellEmpty.r, cellEmpty.g, cellEmpty.b, 255);
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            SDL_FRect rect = {(float)(GAP + x * (TILE_SIZE + GAP)), (float)(GAP + y * (TILE_SIZE + GAP)), (float)TILE_SIZE, (float)TILE_SIZE};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    SDL_SetRenderTarget(renderer, NULL);
}

static void drawGame(SDL_Renderer* renderer, Game* game) {
    updateScoreTextures(renderer, game);
    
    SDL_SetRenderDrawColor(renderer, bgLight.r, bgLight.g, bgLight.b, 255);
    SDL_RenderClear(renderer);

    int gridWidth = (TILE_SIZE + GAP) * GRID_SIZE + GAP;
    int gridHeight = (TILE_SIZE + GAP) * GRID_SIZE + GAP;

    int headerPadding = GAP;
    SDL_FRect headerRect = {headerPadding, headerPadding, (float)(WINDOW_SIZE - headerPadding*2), (float)(HEADER_HEIGHT - headerPadding*2)};
    SDL_SetRenderDrawColor(renderer, 170, 150, 140, 255);
    SDL_RenderFillRect(renderer, &headerRect);

    if (scoreTex) {
        int w, h; SDL_GetTextureSize(scoreTex, &w, &h);
        SDL_RenderTexture(renderer, scoreTex, NULL, &(SDL_FRect){headerPadding + 10, HEADER_HEIGHT/2 - 10, w, h});
    }
    if (bestTex) {
        int w, h; SDL_GetTextureSize(bestTex, &w, &h);
        SDL_RenderTexture(renderer, bestTex, NULL, &(SDL_FRect){WINDOW_SIZE/2 - 30, HEADER_HEIGHT/2 - 10, w, h});
    }
    if (restartTex) {
        int w, h; SDL_GetTextureSize(restartTex, &w, &h);
        SDL_RenderTexture(renderer, restartTex, NULL, &(SDL_FRect){WINDOW_SIZE - headerPadding - 80, HEADER_HEIGHT/2 - 10, w, h});
    }

    if (gridBgTex) {
        SDL_RenderTexture(renderer, gridBgTex, NULL, &(SDL_FRect){gridOffsetX, gridOffsetY, gridWidth, gridHeight});
    }

    for (int i = 0; i < CELL_COUNT; i++) {
        game->tiles[i].x = (float)(i % GRID_SIZE);
        game->tiles[i].y = (float)(i / GRID_SIZE);
    }

    if (game->scoreGained > 0 && plusTex) {
        int w, h; SDL_GetTextureSize(plusTex, &w, &h);
        SDL_RenderTexture(renderer, plusTex, NULL, &(SDL_FRect){130, HEADER_HEIGHT/2 - 10, w, h});
    }

    for (int i = 0; i < CELL_COUNT; i++) {
        drawTile(renderer, &game->tiles[i]);
    }
    
    if (game->gameOverAlpha > 0) {
        int alpha = (int)(180 * game->gameOverAlpha);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
        SDL_FRect overlay = {0, 0, (float)WINDOW_SIZE, (float)WINDOW_SIZE};
        SDL_RenderFillRect(renderer, &overlay);
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    if (!TTF_Init()) {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    font = TTF_OpenFont(getFontPath(), 24.0f);
    if (!font) {
        SDL_Log("TTF_OpenFont failed: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = NULL;
#ifdef __ANDROID__
    window = SDL_CreateWindow("2048", 0, 0, SDL_WINDOW_FULLSCREEN);
#else
    window = SDL_CreateWindow("2048", WINDOW_SIZE, WINDOW_SIZE, 0);
#endif
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    SDL_SetWindowTitle(window, "2048 Game");

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    int screenW, screenH;
    SDL_GetRenderOutputSize(renderer, &screenW, &screenH);
    SDL_Log("Screen size: %d x %d", screenW, screenH);
    
    int minDim = screenW < screenH ? screenW : screenH;
    HEADER_HEIGHT = minDim / 8;
    GAP = minDim / 40;
    TILE_SIZE = (minDim - GAP * (GRID_SIZE + 1)) / GRID_SIZE;
    
    int gridWidth = (TILE_SIZE + GAP) * GRID_SIZE + GAP;
    int gridHeight = (TILE_SIZE + GAP) * GRID_SIZE + GAP;
    WINDOW_SIZE = screenW < screenH ? screenW : screenH;
    gridOffsetX = (screenW - gridWidth) / 2;
    gridOffsetY = HEADER_HEIGHT + GAP + (screenH - HEADER_HEIGHT - gridHeight) / 2;
    
    SDL_Log("Calculated: screenW=%d, screenH=%d, TILE_SIZE=%d, GAP=%d, HEADER=%d", screenW, screenH, TILE_SIZE, GAP, HEADER_HEIGHT);
    SDL_Log("Grid offset: x=%d, y=%d, gridW=%d, gridH=%d", gridOffsetX, gridOffsetY, gridWidth, gridHeight);
    
    initNumberTextures(renderer);
    createGridBg(renderer);
    
    TTF_SetFontSize(font, 18);
    SDL_Surface* surf = TTF_RenderText_Blended(font, "Restart", 7, (SDL_Color){200, 100, 100, 255});
    if (surf) { restartTex = SDL_CreateTextureFromSurface(renderer, surf); SDL_DestroySurface(surf); }
    
    SDL_Log("Game initialized, starting main loop...");

    SDL_Event event;
    int running = 1;
    int frameCount = 0;
    int touchStartX = 0, touchStartY = 0;
    int touchStarted = 0;
    int currentScreenW = screenW;
    int currentScreenH = screenH;

    Game game;
    if (!loadGame(&game)) {
        initGame(&game);
    }

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_TERMINATING) {
                SDL_Log("Quit event received");
                running = 0;
                saveGame(&game);
            } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                SDL_GetRenderOutputSize(renderer, &currentScreenW, &currentScreenH);
                int minDim = currentScreenW < currentScreenH ? currentScreenW : currentScreenH;
                HEADER_HEIGHT = minDim / 8;
                GAP = minDim / 40;
                TILE_SIZE = (minDim - GAP * (GRID_SIZE + 1)) / GRID_SIZE;
                int gridWidth = (TILE_SIZE + GAP) * GRID_SIZE + GAP;
                int gridHeight = (TILE_SIZE + GAP) * GRID_SIZE + GAP;
                gridOffsetX = (currentScreenW - gridWidth) / 2;
                gridOffsetY = HEADER_HEIGHT + GAP + (currentScreenH - HEADER_HEIGHT - gridHeight) / 2;
                createGridBg(renderer);
                SDL_Log("Orientation changed: %d x %d, HEADER=%d", currentScreenW, currentScreenH, HEADER_HEIGHT);
            } else if (event.type == SDL_EVENT_FINGER_DOWN) {
                touchStarted = 1;
                touchStartX = (int)(event.tfinger.x * currentScreenW);
                touchStartY = (int)(event.tfinger.y * currentScreenH);
            } else if (event.type == SDL_EVENT_FINGER_UP && touchStarted) {
                int touchEndX = (int)(event.tfinger.x * currentScreenW);
                int touchEndY = (int)(event.tfinger.y * currentScreenH);
                
                int restartBtnX = currentScreenW - 100;
                if (touchStartX > restartBtnX && touchStartX < currentScreenW && 
                    touchStartY > 5 && touchStartY < HEADER_HEIGHT - 5) {
                    initGame(&game);
                    game.gameOver = 0;
                    game.gameOverAlpha = 0;
                    saveGame(&game);
                    touchStarted = 0;
                    continue;
                }
                
                int dx = touchEndX - touchStartX;
                int dy = touchEndY - touchStartY;
                int moved = 0;
                int minDist = currentScreenW / 10;
                if (minDist < 30) minDist = 30;
                
                if (game.inputBlocked) {
                    touchStarted = 0;
                    continue;
                }
                
                if (abs(dx) > abs(dy)) {
                    if (dx > minDist) moved = moveTiles(&game, 1, 0);
                    else if (dx < -minDist) moved = moveTiles(&game, -1, 0);
                } else {
                    if (dy > minDist) moved = moveTiles(&game, 0, 1);
                    else if (dy < -minDist) moved = moveTiles(&game, 0, -1);
                }
                
                if (moved) {
                    spawnTile(&game);
                    if (checkGameOver(&game)) {
                        game.gameOver = 1;
                    }
                    saveGame(&game);
                }
                touchStarted = 0;
            } else if (event.type == SDL_EVENT_FINGER_UP && (game.gameOver || game.gameOverAlpha > 0)) {
                initGame(&game);
                game.gameOver = 0;
                game.gameOverAlpha = 0;
                saveGame(&game);
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (game.inputBlocked) continue;
                
                int moved = 0;
                switch (event.key.scancode) {
                    case SDL_SCANCODE_UP:
                        moved = moveTiles(&game, 0, -1);
                        break;
                    case SDL_SCANCODE_DOWN:
                        moved = moveTiles(&game, 0, 1);
                        break;
                    case SDL_SCANCODE_LEFT:
                        moved = moveTiles(&game, -1, 0);
                        break;
                    case SDL_SCANCODE_RIGHT:
                        moved = moveTiles(&game, 1, 0);
                        break;
                    case SDL_SCANCODE_R:
                        initGame(&game);
                        moved = 1;
                        break;
                    case SDL_SCANCODE_ESCAPE:
                        running = 0;
                        break;
                    default:
                        break;
                }

                if (moved) {
                    spawnTile(&game);
                    if (checkGameOver(&game)) {
                        game.gameOver = 1;
                    }
                    saveGame(&game);
                }
            }
        }

        updateAnimations(&game);
        drawGame(renderer, &game);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    TTF_CloseFont(font);
    cleanupNumberTextures();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    
    SDL_Log("Game exited normally");

    return 0;
}
