#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GRID_SIZE 8
#define HALF_GRID 4
#define CELL_COUNT 64
#define COLLECT_TIME 30
#define TURN_TIME 5

enum { PHASE_COLLECT, PHASE_BATTLE };
enum { MODE_AUTO, MODE_MANUAL, MODE_TURN };
enum { PLAYER_RED, PLAYER_BLUE };
enum { STATE_PLAYING, STATE_TURN_SWAP, STATE_GAME_OVER, STATE_START };

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
    int blockCount;
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
    int screenW, screenH;
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
static SDL_Color redZoneColor = {255, 100, 100, 40};
static SDL_Color blueZoneColor = {100, 100, 255, 40};
static SDL_Color textLight = {255, 255, 255, 255};
static SDL_Color textDark = {119, 110, 101, 255};
static SDL_Color gridLineColor = {205, 193, 180, 255};

static int getEmptyCellInZone(Game* game, int player, int startRow, int endRow) {
    int available[32];
    int count = 0;
    
    for (int y = startRow; y < endRow; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            int found = 0;
            for (int i = 0; i < CELL_COUNT; i++) {
                if (game->red.blocks[i].active && (int)game->red.blocks[i].x == x && (int)game->red.blocks[i].y == y) found = 1;
                if (game->blue.blocks[i].active && (int)game->blue.blocks[i].x == x && (int)game->blue.blocks[i].y == y) found = 1;
            }
            if (!found) available[count++] = y * GRID_SIZE + x;
        }
    }
    
    if (count == 0) return -1;
    return available[rand() % count];
}

static void spawnBlock(Game* game, int player) {
    int startRow = (player == PLAYER_RED) ? 0 : HALF_GRID;
    int endRow = (player == PLAYER_RED) ? HALF_GRID : GRID_SIZE;
    
    int pos = getEmptyCellInZone(game, player, startRow, endRow);
    if (pos < 0) return;
    
    int value = (rand() % 10 < 9) ? 2 : 4;
    
    if (player == PLAYER_RED) {
        for (int i = 0; i < CELL_COUNT; i++) {
            if (!game->red.blocks[i].active) {
                game->red.blocks[i].value = value;
                game->red.blocks[i].x = pos % GRID_SIZE;
                game->red.blocks[i].y = pos / GRID_SIZE;
                game->red.blocks[i].owner = PLAYER_RED;
                game->red.blocks[i].active = 1;
                game->red.blockCount++;
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
                game->blue.blockCount++;
                break;
            }
        }
    }
}

static int moveBlocksInZone(Game* game, int player, int dx, int dy, int startRow, int endRow) {
    Player* p = (player == PLAYER_RED) ? &game->red : &game->blue;
    int moved = 0;
    
    if (dy != 0) {
        int startY = (dy > 0) ? endRow - 1 : startRow;
        int endY = (dy > 0) ? startRow - 1 : endRow;
        int stepY = (dy > 0) ? -1 : 1;
        
        for (int y = startY; y != endY; y += stepY) {
            for (int x = 0; x < GRID_SIZE; x++) {
                for (int i = 0; i < CELL_COUNT; i++) {
                    if (!p->blocks[i].active) continue;
                    if ((int)p->blocks[i].x != x) continue;
                    if ((int)p->blocks[i].y != y) continue;
                    if (y < startRow || y >= endRow) continue;
                    
                    int newY = y;
                    while (1) {
                        int nextY = newY + dy;
                        if (nextY < startRow || nextY >= endRow) break;
                        
                        int blocked = 0;
                        for (int j = 0; j < CELL_COUNT; j++) {
                            if (!p->blocks[j].active) continue;
                            if ((int)p->blocks[j].x == x && (int)p->blocks[j].y == nextY) {
                                blocked = 1;
                                if (p->blocks[j].value == p->blocks[i].value && p->blocks[j].value < 2048) {
                                    p->blocks[j].value *= 2;
                                    p->score += p->blocks[j].value;
                                    p->blocks[i].active = 0;
                                    p->blockCount--;
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
            for (int y = startRow; y < endRow; y++) {
                for (int i = 0; i < CELL_COUNT; i++) {
                    if (!p->blocks[i].active) continue;
                    if ((int)p->blocks[i].x != x) continue;
                    if ((int)p->blocks[i].y != y) continue;
                    if (y < startRow || y >= endRow) continue;
                    
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
                                    p->blockCount--;
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

static int moveBlocks(Game* game, int player, int dx, int dy) {
    return moveBlocksInZone(game, player, dx, dy, 0, GRID_SIZE);
}

static int moveBlocksFullBoard(Game* game, int player, int dx, int dy) {
    return moveBlocksInZone(game, player, dx, dy, 0, GRID_SIZE);
}

static void updateBattleAuto(Game* game) {
    moveBlocksInZone(game, PLAYER_RED, 0, 1, 0, GRID_SIZE);
    moveBlocksInZone(game, PLAYER_BLUE, 0, -1, 0, GRID_SIZE);
    
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
                    game->blue.blockCount--;
                } else if (bv > rv) {
                    game->blue.blocks[bi].value = bv + rv;
                    game->blue.score += rv;
                    game->red.blocks[ri].active = 0;
                    game->red.blockCount--;
                } else {
                    game->red.blocks[ri].active = 0;
                    game->blue.blocks[bi].active = 0;
                    game->red.blockCount--;
                    game->blue.blockCount--;
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
    game->state = STATE_START;
    game->screenW = 800;
    game->screenH = 800;
}

static void initRound(Game* game) {
    memset(&game->red, 0, sizeof(Player));
    memset(&game->blue, 0, sizeof(Player));
    game->phase = PHASE_COLLECT;
    game->timer = COLLECT_TIME;
    game->state = STATE_PLAYING;
    game->winner = -1;
    game->activePlayer = PLAYER_RED;
    game->turnTimer = TURN_TIME;
    
    spawnBlock(game, PLAYER_RED);
    spawnBlock(game, PLAYER_RED);
    spawnBlock(game, PLAYER_BLUE);
    spawnBlock(game, PLAYER_BLUE);
}

static void initMatch(Game* game) {
    game->red.roundWins = 0;
    game->blue.roundWins = 0;
    game->roundNumber = 1;
    game->matchWinner = -1;
    initRound(game);
}

static void drawBlock(SDL_Renderer* renderer, Block* block, int x, int y, Uint32* colors) {
    if (!block->active) return;
    
    int log2 = 0, v = block->value;
    while (v > 1) { v >>= 1; log2++; }
    if (log2 >= 11) log2 = 11;
    
    Uint32 color = colors[log2];
    SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 255);
    
    int size = TILE_SIZE - 4;
    SDL_FRect rect = {(float)(x + 2), (float)(y + 2), (float)size, (float)size};
    SDL_RenderFillRect(renderer, &rect);
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
    initMatch(&game);
    
    int running = 1;
    int touchStartX = 0, touchStartY = 0;
    int touchStarted = 0;
    int lastTick = SDL_GetTicks();
    int lastTick100 = SDL_GetTicks();
    int modeSelection = 0;
    
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_TERMINATING) {
                running = 0;
            } else if (event.type == SDL_EVENT_FINGER_DOWN) {
                touchStarted = 1;
                touchStartX = (int)(event.tfinger.x * game.screenW);
                touchStartY = (int)(event.tfinger.y * game.screenH);
            } else if (event.type == SDL_EVENT_FINGER_UP && touchStarted) {
                int touchEndX = (int)(event.tfinger.x * game.screenW);
                int touchEndY = (int)(event.tfinger.y * game.screenH);
                int dx = touchEndX - touchStartX;
                int dy = touchEndY - touchStartY;
                int minDist = game.screenW / 15;
                if (minDist < 30) minDist = 30;
                
                if (game.state == STATE_START) {
                    modeSelection = (modeSelection + 1) % 3;
                    touchStarted = 0;
                    continue;
                }
                
                if (game.state == STATE_GAME_OVER) {
                    if (game.matchWinner >= 0) {
                        initMatch(&game);
                    } else {
                        initRound(&game);
                    }
                    touchStarted = 0;
                    continue;
                }
                
                if (game.state != STATE_PLAYING) {
                    touchStarted = 0;
                    continue;
                }
                
                if (game.phase == PHASE_COLLECT) {
                    if (touchStartY < game.screenH / 2) {
                        if (abs(dx) > abs(dy)) {
                            if (dx > minDist) { moveBlocksInZone(&game, PLAYER_RED, 1, 0, 0, HALF_GRID); spawnBlock(&game, PLAYER_RED); }
                            else if (dx < -minDist) { moveBlocksInZone(&game, PLAYER_RED, -1, 0, 0, HALF_GRID); spawnBlock(&game, PLAYER_RED); }
                        } else {
                            if (dy > minDist) { moveBlocksInZone(&game, PLAYER_RED, 0, 1, 0, HALF_GRID); spawnBlock(&game, PLAYER_RED); }
                            else if (dy < -minDist) { moveBlocksInZone(&game, PLAYER_RED, 0, -1, 0, HALF_GRID); spawnBlock(&game, PLAYER_RED); }
                        }
                    } else {
                        if (abs(dx) > abs(dy)) {
                            if (dx > minDist) { moveBlocksInZone(&game, PLAYER_BLUE, 1, 0, HALF_GRID, GRID_SIZE); spawnBlock(&game, PLAYER_BLUE); }
                            else if (dx < -minDist) { moveBlocksInZone(&game, PLAYER_BLUE, -1, 0, HALF_GRID, GRID_SIZE); spawnBlock(&game, PLAYER_BLUE); }
                        } else {
                            if (dy > minDist) { moveBlocksInZone(&game, PLAYER_BLUE, 0, 1, HALF_GRID, GRID_SIZE); spawnBlock(&game, PLAYER_BLUE); }
                            else if (dy < -minDist) { moveBlocksInZone(&game, PLAYER_BLUE, 0, -1, HALF_GRID, GRID_SIZE); spawnBlock(&game, PLAYER_BLUE); }
                        }
                    }
                } else if (game.phase == PHASE_BATTLE) {
                    if (game.mode == MODE_AUTO) {
                    } else if (game.mode == MODE_MANUAL) {
                        if (touchStartY < game.screenH / 2) {
                            if (abs(dx) > abs(dy)) {
                                if (dx > minDist) moveBlocksFullBoard(&game, PLAYER_RED, 1, 0);
                                else if (dx < -minDist) moveBlocksFullBoard(&game, PLAYER_RED, -1, 0);
                            } else {
                                if (dy > minDist) moveBlocksFullBoard(&game, PLAYER_RED, 0, 1);
                                else if (dy < -minDist) moveBlocksFullBoard(&game, PLAYER_RED, 0, -1);
                            }
                        } else {
                            if (abs(dx) > abs(dy)) {
                                if (dx > minDist) moveBlocksFullBoard(&game, PLAYER_BLUE, 1, 0);
                                else if (dx < -minDist) moveBlocksFullBoard(&game, PLAYER_BLUE, -1, 0);
                            } else {
                                if (dy > minDist) moveBlocksFullBoard(&game, PLAYER_BLUE, 0, 1);
                                else if (dy < -minDist) moveBlocksFullBoard(&game, PLAYER_BLUE, 0, -1);
                            }
                        }
                    } else if (game.mode == MODE_TURN) {
                        int canMove = 0;
                        if (game.activePlayer == PLAYER_RED && touchStartY < game.screenH / 2) canMove = 1;
                        if (game.activePlayer == PLAYER_BLUE && touchStartY >= game.screenH / 2) canMove = 1;
                        
                        if (canMove) {
                            if (abs(dx) > abs(dy)) {
                                if (dx > minDist) { moveBlocksFullBoard(&game, game.activePlayer, 1, 0); game.activePlayer = 1 - game.activePlayer; game.turnTimer = TURN_TIME; }
                                else if (dx < -minDist) { moveBlocksFullBoard(&game, game.activePlayer, -1, 0); game.activePlayer = 1 - game.activePlayer; game.turnTimer = TURN_TIME; }
                            } else {
                                if (dy > minDist) { moveBlocksFullBoard(&game, game.activePlayer, 0, 1); game.activePlayer = 1 - game.activePlayer; game.turnTimer = TURN_TIME; }
                                else if (dy < -minDist) { moveBlocksFullBoard(&game, game.activePlayer, 0, -1); game.activePlayer = 1 - game.activePlayer; game.turnTimer = TURN_TIME; }
                            }
                        }
                    }
                }
                
                touchStarted = 0;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) running = 0;
                if (event.key.scancode == SDL_SCANCODE_R) initMatch(&game);
                if (event.key.scancode == SDL_SCANCODE_1) game.mode = MODE_AUTO;
                if (event.key.scancode == SDL_SCANCODE_2) game.mode = MODE_MANUAL;
                if (event.key.scancode == SDL_SCANCODE_3) game.mode = MODE_TURN;
                if (event.key.scancode == SDL_SCANCODE_SPACE && game.state == STATE_START) {
                    initRound(&game);
                }
            } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                SDL_GetRendererOutputSize(renderer, &game.screenW, &game.screenH);
            }
        }
        
        int now = SDL_GetTicks();
        
        if (now - lastTick100 >= 100) {
            lastTick100 = now;
            
            if (game.state == STATE_PLAYING) {
                if (game.phase == PHASE_COLLECT) {
                    game.timer--;
                    if (game.timer <= 0) {
                        game.phase = PHASE_BATTLE;
                        game.timer = 0;
                        
                        if (game.mode == MODE_TURN) {
                            int redScore = 0, blueScore = 0;
                            for (int i = 0; i < CELL_COUNT; i++) {
                                if (game.red.blocks[i].active) redScore += game.red.blocks[i].value;
                                if (game.blue.blocks[i].active) blueScore += game.blue.blocks[i].value;
                            }
                            game.activePlayer = (redScore < blueScore) ? PLAYER_RED : PLAYER_BLUE;
                            game.turnTimer = TURN_TIME;
                        }
                    }
                } else if (game.phase == PHASE_BATTLE) {
                    if (game.mode == MODE_AUTO) {
                        updateBattleAuto(&game);
                        int w = checkBattleOver(&game);
                        if (w >= 0 || w == 2) {
                            game.winner = w;
                            game.state = STATE_GAME_OVER;
                            if (w == PLAYER_RED) game.red.roundWins++;
                            else if (w == PLAYER_BLUE) game.blue.roundWins++;
                            if (game.red.roundWins >= 2) game.matchWinner = PLAYER_RED;
                            else if (game.blue.roundWins >= 2) game.matchWinner = PLAYER_BLUE;
                        }
                    } else if (game.mode == MODE_TURN) {
                        game.turnTimer -= 0.1f;
                        if (game.turnTimer <= 0) {
                            game.activePlayer = 1 - game.activePlayer;
                            game.turnTimer = TURN_TIME;
                        }
                    }
                }
            }
        }
        
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, 255);
        SDL_RenderClear(renderer);
        
        int minDim = game.screenW < game.screenH ? game.screenW : game.screenH;
        TILE_SIZE = minDim / 10;
        GAP = TILE_SIZE / 10;
        
        int gw = TILE_SIZE * GRID_SIZE + GAP * (GRID_SIZE + 1);
        int gh = TILE_SIZE * GRID_SIZE + GAP * (GRID_SIZE + 1);
        gridOffsetX = (game.screenW - gw) / 2;
        gridOffsetY = (game.screenH - gh) / 2;
        
        if (game.state == STATE_START) {
            const char* title = "MERGE & DEVOUR";
            const char* modes[] = {"AUTO MODE", "MANUAL MODE", "TURN-BASED"};
            SDL_Log("Mode selected: %s", modes[game.mode]);
            SDL_RenderPresent(renderer);
            SDL_Delay(16);
            continue;
        }
        
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
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
            SDL_FRect barrier = {(float)gridOffsetX, (float)(gridOffsetY + gh/2 - 3), (float)gw, 6};
            SDL_RenderFillRect(renderer, &barrier);
        }
        
        for (int i = 0; i < CELL_COUNT; i++) {
            if (game.red.blocks[i].active) {
                int x = gridOffsetX + GAP + (int)game.red.blocks[i].x * (TILE_SIZE + GAP);
                int y = gridOffsetY + GAP + (int)game.red.blocks[i].y * (TILE_SIZE + GAP);
                drawBlock(renderer, &game.red.blocks[i], x, y, redBlockColors);
            }
            
            if (game.blue.blocks[i].active) {
                int x = gridOffsetX + GAP + (int)game.blue.blocks[i].x * (TILE_SIZE + GAP);
                int y = gridOffsetY + GAP + (int)game.blue.blocks[i].y * (TILE_SIZE + GAP);
                drawBlock(renderer, &game.blue.blocks[i], x, y, blueBlockColors);
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
