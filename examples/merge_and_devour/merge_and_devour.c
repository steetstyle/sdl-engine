#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define GRID_SIZE 8
#define HALF_GRID 4
#define CELL_COUNT 64
#define COLLECT_TIME 30
#define TURN_TIME 5

enum { PHASE_COLLECT, PHASE_BATTLE };
enum { MODE_AUTO, MODE_MANUAL, MODE_TURN };
enum { PLAYER_RED, PLAYER_BLUE };
enum { STATE_PLAYING, STATE_TURN_SWAP, STATE_GAME_OVER };

typedef struct {
    int value;
    float x, y;
    int owner;
    int active;
} Block;

typedef struct {
    Block blocks[CELL_COUNT];
    int score;
    int roundWins;
    int stars;
} Player;

typedef struct {
    Player red;
    Player blue;
    int phase;
    int mode;
    int state;
    int timer;
    float turnTimer;
    int activePlayer;
    int roundNumber;
    int winner;
    int matchWinner;
    int inputBlocked;
} Game;

static int gridOffsetX, gridOffsetY;
static int TILE_SIZE, GAP;

static Uint32 redBlockColors[12] = {
    0xFFEE4D2, 0xFFF2B179, 0xFFF59563, 0xFFF67C5F,
    0xFFF65E3B, 0xFFEDCF72, 0xFFEDCC61, 0xFFEDC850,
    0xFFEDC53F, 0xFFEDC22E, 0xFF8F7A6E, 0xFF8F7A6E
};

static Uint32 blueBlockColors[12] = {
    0xFF6BB3FF, 0xFF5CA3FF, 0xFF4D93FF, 0xFF3D83FF,
    0xFF2E73FF, 0xFF1F63FF, 0xFF0F53FF, 0xFF0043FF,
    0xFF0033AF, 0xFF00237F, 0xFF00134F, 0xFF00001F
};

static SDL_Color bgColor = {187, 173, 160, 255};
static SDL_Color redZoneColor = {255, 200, 200, 50};
static SDL_Color blueZoneColor = {200, 200, 255, 50};
static SDL_Color textLight = {255, 255, 255, 255};
static SDL_Color textDark = {119, 110, 101, 255};
static SDL_Color gridLineColor = {205, 193, 180, 255};

static int getEmptyCell(Game* game, int player) {
    int startRow = (player == PLAYER_RED) ? 0 : HALF_GRID;
    int endRow = (player == PLAYER_RED) ? HALF_GRID : GRID_SIZE;
    
    int available[32];
    int count = 0;
    
    for (int y = startRow; y < endRow; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            int idx = y * GRID_SIZE + x;
            int found = 0;
            for (int i = 0; i < CELL_COUNT; i++) {
                if (game->red.blocks[i].active && (int)game->red.blocks[i].x == x && (int)game->red.blocks[i].y == y) found = 1;
                if (game->blue.blocks[i].active && (int)game->blue.blocks[i].x == x && (int)game->blue.blocks[i].y == y) found = 1;
            }
            if (!found) available[count++] = idx;
        }
    }
    
    if (count == 0) return -1;
    return available[rand() % count];
}

static void spawnBlock(Game* game, int player) {
    int startRow = (player == PLAYER_RED) ? 0 : HALF_GRID;
    int endRow = (player == PLAYER_RED) ? HALF_GRID : GRID_SIZE;
    
    int available[32];
    int count = 0;
    
    for (int y = startRow; y < endRow; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            int idx = y * GRID_SIZE + x;
            int found = 0;
            for (int i = 0; i < CELL_COUNT; i++) {
                if (game->red.blocks[i].active && (int)game->red.blocks[i].x == x && (int)game->red.blocks[i].y == y) found = 1;
                if (game->blue.blocks[i].active && (int)game->blue.blocks[i].x == x && (int)game->blue.blocks[i].y == y) found = 1;
            }
            if (!found) available[count++] = idx;
        }
    }
    
    if (count == 0) return;
    
    int pos = available[rand() % count];
    int value = (rand() % 10 < 9) ? 2 : 4;
    
    if (player == PLAYER_RED) {
        for (int i = 0; i < CELL_COUNT; i++) {
            if (!game->red.blocks[i].active) {
                game->red.blocks[i].value = value;
                game->red.blocks[i].x = pos % GRID_SIZE;
                game->red.blocks[i].y = pos / GRID_SIZE;
                game->red.blocks[i].owner = PLAYER_RED;
                game->red.blocks[i].active = 1;
                break;
            }
        }
    } else {
        for (int i = 0; i < CELL_COUNT; i++) {
            if (!game->blue.blocks[i].active) {
                game->blue.blocks[i].value = value;
                game->blue.blocks[i].x = pos % GRID_SIZE;
                game->blue.blocks[i].y = pos / GRID_SIZE;
                game->blue.blocks[i].owner = PLAYER_BLUE;
                game->blue.blocks[i].active = 1;
                break;
            }
        }
    }
}

static int moveBlocks(Game* game, int player, int dx, int dy) {
    Player* p = (player == PLAYER_RED) ? &game->red : &game->blue;
    int moved = 0;
    
    if (dy != 0) {
        int startY = (dy > 0) ? GRID_SIZE - 1 : 0;
        int endY = (dy > 0) ? -1 : GRID_SIZE;
        int stepY = (dy > 0) ? -1 : 1;
        
        for (int y = startY; y != endY; y += stepY) {
            for (int x = 0; x < GRID_SIZE; x++) {
                for (int i = 0; i < CELL_COUNT; i++) {
                    if (!p->blocks[i].active) continue;
                    if ((int)p->blocks[i].x != x) continue;
                    if ((int)p->blocks[i].y != y) continue;
                    
                    int newY = y;
                    while (1) {
                        int nextY = newY + dy;
                        if (nextY < 0 || nextY >= GRID_SIZE) break;
                        
                        int blocked = 0;
                        for (int j = 0; j < CELL_COUNT; j++) {
                            if (!p->blocks[j].active) continue;
                            if ((int)p->blocks[j].x == x && (int)p->blocks[j].y == nextY) {
                                blocked = 1;
                                if (p->blocks[j].value == p->blocks[i].value && p->blocks[j].value < 2048) {
                                    p->blocks[j].value *= 2;
                                    p->score += p->blocks[j].value;
                                    p->blocks[i].active = 0;
                                    moved = 1;
                                }
                                break;
                            }
                        }
                        if (blocked) break;
                        newY = nextY;
                    }
                    
                    if (newY != y) {
                        p->blocks[i].y = (float)newY;
                        moved = 1;
                    }
                }
            }
        }
    }
    
    if (dx != 0) {
        int startX = (dx > 0) ? GRID_SIZE - 1 : 0;
        int endX = (dx > 0) ? -1 : GRID_SIZE;
        int stepX = (dx > 0) ? -1 : 1;
        
        for (int x = startX; x != endX; x += stepX) {
            for (int y = 0; y < GRID_SIZE; y++) {
                for (int i = 0; i < CELL_COUNT; i++) {
                    if (!p->blocks[i].active) continue;
                    if ((int)p->blocks[i].x != x) continue;
                    if ((int)p->blocks[i].y != y) continue;
                    
                    int newX = x;
                    while (1) {
                        int nextX = newX + dx;
                        if (nextX < 0 || nextX >= GRID_SIZE) break;
                        
                        int blocked = 0;
                        for (int j = 0; j < CELL_COUNT; j++) {
                            if (!p->blocks[j].active) continue;
                            if ((int)p->blocks[j].x == nextX && (int)p->blocks[j].y == y) {
                                blocked = 1;
                                if (p->blocks[j].value == p->blocks[i].value && p->blocks[j].value < 2048) {
                                    p->blocks[j].value *= 2;
                                    p->score += p->blocks[j].value;
                                    p->blocks[i].active = 0;
                                    moved = 1;
                                }
                                break;
                            }
                        }
                        if (blocked) break;
                        newX = nextX;
                    }
                    
                    if (newX != x) {
                        p->blocks[i].x = (float)newX;
                        moved = 1;
                    }
                }
            }
        }
    }
    
    return moved;
}

static void updateBattleAuto(Game* game) {
    moveBlocks(game, PLAYER_RED, 0, 1);
    moveBlocks(game, PLAYER_BLUE, 0, -1);
    
    for (int ri = 0; ri < CELL_COUNT; ri++) {
        if (!game->red.blocks[ri].active) continue;
        for (int bi = 0; bi < CELL_COUNT; bi++) {
            if (!game->blue.blocks[bi].active) continue;
            
            if ((int)game->red.blocks[ri].x == (int)game->blue.blocks[bi].x &&
                (int)game->red.blocks[ri].y == (int)game->blue.blocks[bi].y) {
                
                int rv = game->red.blocks[ri].value;
                int bv = game->blue.blocks[bi].value;
                
                if (rv > bv) {
                    game->red.blocks[ri].value = rv + bv;
                    game->red.score += bv;
                    game->blue.blocks[bi].active = 0;
                } else if (bv > rv) {
                    game->blue.blocks[bi].value = bv + rv;
                    game->blue.score += rv;
                    game->red.blocks[ri].active = 0;
                } else {
                    game->red.blocks[ri].active = 0;
                    game->blue.blocks[bi].active = 0;
                }
            }
        }
    }
}

static int checkBattleOver(Game* game) {
    int redActive = 0, blueActive = 0;
    
    for (int i = 0; i < CELL_COUNT; i++) {
        if (game->red.blocks[i].active) redActive = 1;
        if (game->blue.blocks[i].active) blueActive = 1;
    }
    
    if (!redActive && !blueActive) return 2;
    if (!redActive) return PLAYER_BLUE;
    if (!blueActive) return PLAYER_RED;
    return -1;
}

static void initGame(Game* game) {
    memset(game, 0, sizeof(Game));
    game->phase = PHASE_COLLECT;
    game->mode = MODE_AUTO;
    game->timer = COLLECT_TIME;
    game->roundNumber = 1;
    game->state = STATE_PLAYING;
}

static void initRound(Game* game) {
    memset(&game->red, 0, sizeof(Player));
    memset(&game->blue, 0, sizeof(Player));
    game->phase = PHASE_COLLECT;
    game->timer = COLLECT_TIME;
    game->state = STATE_PLAYING;
    game->winner = -1;
    
    spawnBlock(game, PLAYER_RED);
    spawnBlock(game, PLAYER_RED);
    spawnBlock(game, PLAYER_BLUE);
    spawnBlock(game, PLAYER_BLUE);
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    if (!SDL_Init(SDL_INIT_VIDEO)) return 1;
    
    SDL_Window* window = SDL_CreateWindow("Merge & Devour", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    
    srand((unsigned int)time(NULL));
    
    Game game;
    initGame(&game);
    initRound(&game);
    
    int running = 1;
    int touchStartX = 0, touchStartY = 0;
    int touchStarted = 0;
    int lastTick = SDL_GetTicks();
    
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_TERMINATING) {
                running = 0;
            } else if (event.type == SDL_EVENT_FINGER_DOWN) {
                touchStarted = 1;
                touchStartX = (int)(event.tfinger.x * 800);
                touchStartY = (int)(event.tfinger.y * 800);
            } else if (event.type == SDL_EVENT_FINGER_UP && touchStarted) {
                int touchEndX = (int)(event.tfinger.x * 800);
                int touchEndY = (int)(event.tfinger.y * 800);
                int dx = touchEndX - touchStartX;
                int dy = touchEndY - touchStartY;
                int minDist = 30;
                
                if (game.state != STATE_PLAYING) {
                    initRound(&game);
                    touchStarted = 0;
                    continue;
                }
                
                if (game.phase == PHASE_COLLECT) {
                    if (touchStartY < 400) {
                        if (abs(dx) > abs(dy)) {
                            if (dx > minDist) moveBlocks(&game, PLAYER_RED, 1, 0);
                            else if (dx < -minDist) moveBlocks(&game, PLAYER_RED, -1, 0);
                        } else {
                            if (dy > minDist) moveBlocks(&game, PLAYER_RED, 0, 1);
                            else if (dy < -minDist) moveBlocks(&game, PLAYER_RED, 0, -1);
                        }
                    } else {
                        if (abs(dx) > abs(dy)) {
                            if (dx > minDist) moveBlocks(&game, PLAYER_BLUE, 1, 0);
                            else if (dx < -minDist) moveBlocks(&game, PLAYER_BLUE, -1, 0);
                        } else {
                            if (dy > minDist) moveBlocks(&game, PLAYER_BLUE, 0, 1);
                            else if (dy < -minDist) moveBlocks(&game, PLAYER_BLUE, 0, -1);
                        }
                    }
                }
                
                touchStarted = 0;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) running = 0;
                if (event.key.scancode == SDL_SCANCODE_R) initRound(&game);
            }
        }
        
        int now = SDL_GetTicks();
        if (now - lastTick >= 1000) {
            lastTick = now;
            if (game.state == STATE_PLAYING) {
                if (game.phase == PHASE_COLLECT) {
                    game.timer--;
                    if (game.timer <= 0) {
                        game.phase = PHASE_BATTLE;
                        game.timer = 0;
                    }
                } else if (game.phase == PHASE_BATTLE && game.mode == MODE_AUTO) {
                    updateBattleAuto(&game);
                    int w = checkBattleOver(&game);
                    if (w >= 0 || w == 2) {
                        game.winner = w;
                        game.state = STATE_GAME_OVER;
                        if (w == PLAYER_RED) game.red.roundWins++;
                        else if (w == PLAYER_BLUE) game.blue.roundWins++;
                    }
                }
            }
        }
        
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, 255);
        SDL_RenderClear(renderer);
        
        int gw = TILE_SIZE * GRID_SIZE + GAP * (GRID_SIZE + 1);
        int gh = TILE_SIZE * GRID_SIZE + GAP * (GRID_SIZE + 1);
        gridOffsetX = (800 - gw) / 2;
        gridOffsetY = (800 - gh) / 2;
        
        SDL_SetRenderDrawColor(renderer, redZoneColor.r, redZoneColor.g, redZoneColor.b, redZoneColor.a);
        SDL_FRect redZone = {(float)gridOffsetX, (float)gridOffsetY, (float)gw, (float)gh/2};
        SDL_RenderFillRect(renderer, &redZone);
        
        SDL_SetRenderDrawColor(renderer, blueZoneColor.r, blueZoneColor.g, blueZoneColor.b, blueZoneColor.a);
        SDL_FRect blueZone = {(float)gridOffsetX, (float)gridOffsetY + gh/2, (float)gw, (float)gh/2};
        SDL_RenderFillRect(renderer, &blueZone);
        
        SDL_SetRenderDrawColor(renderer, gridLineColor.r, gridLineColor.g, gridLineColor.b, 255);
        for (int y = 0; y <= GRID_SIZE; y++) {
            for (int x = 0; x <= GRID_SIZE; x++) {
                SDL_FRect rect = {
                    (float)(gridOffsetX + x * (TILE_SIZE + GAP)),
                    (float)(gridOffsetY + y * (TILE_SIZE + GAP)),
                    (float)TILE_SIZE,
                    (float)TILE_SIZE
                };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
        
        if (game.phase == PHASE_COLLECT) {
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_FRect barrier = {(float)gridOffsetX, (float)(gridOffsetY + gh/2 - 5), (float)gw, 10};
            SDL_RenderFillRect(renderer, &barrier);
        }
        
        for (int i = 0; i < CELL_COUNT; i++) {
            if (game.red.blocks[i].active) {
                int x = gridOffsetX + GAP + (int)game.red.blocks[i].x * (TILE_SIZE + GAP);
                int y = gridOffsetY + GAP + (int)game.red.blocks[i].y * (TILE_SIZE + GAP);
                int log2 = 0, v = game.red.blocks[i].value;
                while (v > 1) { v >>= 1; log2++; }
                if (log2 >= 11) log2 = 11;
                Uint32 color = redBlockColors[log2];
                SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 255);
                SDL_FRect rect = {(float)x, (float)y, (float)TILE_SIZE, (float)TILE_SIZE};
                SDL_RenderFillRect(renderer, &rect);
            }
            
            if (game.blue.blocks[i].active) {
                int x = gridOffsetX + GAP + (int)game.blue.blocks[i].x * (TILE_SIZE + GAP);
                int y = gridOffsetY + GAP + (int)game.blue.blocks[i].y * (TILE_SIZE + GAP);
                int log2 = 0, v = game.blue.blocks[i].value;
                while (v > 1) { v >>= 1; log2++; }
                if (log2 >= 11) log2 = 11;
                Uint32 color = blueBlockColors[log2];
                SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 255);
                SDL_FRect rect = {(float)x, (float)y, (float)TILE_SIZE, (float)TILE_SIZE};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
