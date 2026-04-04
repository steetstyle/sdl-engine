# 2048 Augmented Gameplay Plan

## 1. Core Gameplay Enhancements

### 1.1 Undo System
- Store last 3-5 game states in memory
- Allow undo with swipe down gesture or button
- Cost: Points decrease with each undo

### 1.2 Streak Multiplier
- Consecutive moves without spawning new tile = multiplier
- Multiplier: 1x → 2x → 4x → 8x
- Reset on spawn new tile
- Display current multiplier prominently

## 2. Power-Ups (Collectible or Earned)

### 2.1 Freeze Tile
- Tap to freeze a tile for 1 move
- Frozen tiles cannot merge
- Visual: Blue border indicator

### 2.2 Shuffle
- Randomize all tile positions
- Keep values same
- Useful when stuck

### 2.3 Remove
- Remove one specific tile
- Free up space strategically

### 2.4 Double
- Double a selected tile's value immediately

### 2.5 Bomb
- Clear 3x3 area around tapped tile

## 3. Game Modes

### 3.1 Classic
- Original 2048 rules
- Best score tracking

### 3.2 Time Attack
- 60-second timer
- Score as high as possible
- Add +10 seconds per 2048 tile reached

### 3.3 Endless
- No game over until no moves
- Track total moves and time

### 3.4 Challenge Daily
- Fixed starting board
- Achieve target score in limited moves
- Share results

## 4. Scoring System

### 4.1 Base Points
- 2→4: 2 points
- 4→8: 4 points
- 8→16: 8 points
- (Doubles each merge)

### 4.2 Bonus Points
- Streak multiplier: 1x-8x
- No-undo bonus: +50%
- Speed bonus: Faster moves = more points

## 5. Visual & Audio

### 5.1 Tile Themes
- Classic (current)
- Neon (glowing edges)
- Minimal (solid colors)
- Material (shadows)

### 5.2 Effects
- Merge: Particle burst
- New tile spawn: Pop-in animation
- Milestone (1024, 2048): Screen flash

### 5.3 Sound (Optional)
- Move: Soft slide
- Merge: Click/pop
- Game over: Subtle tone

## 6. Progression

### 6.1 Daily Streak
- Play at least once daily
- Bonus coins for 7-day streak

### 6.2 Achievements
- "Reach 2048 in under 100 moves"
- "Score over 10,000"
- "5x multiplier for 10 moves"

### 6.3 Statistics
- Games played
- Best score
- Average moves per game
- Win rate

## 7. Technical Implementation

### 7.1 State Management
```
GameState:
  - board[4][4]: int
  - score: int
  - bestScore: int
  - multiplier: int
  - movesWithoutSpawn: int
  - powerups: {freeze: int, shuffle: int, ...}
  - history: Stack<GameState> (max 5)
```

### 7.2 Persistence
- Save: score, bestScore, dailyStreak, achievements
- Don't save powerups (encourage replay)

### 7.3 Performance
- Pre-render all tile textures
- Cache grid background
- No animations (per prior request)
- 60fps target

## Implementation Priority

1. **Phase 1** - Streak multiplier + scoring changes
2. **Phase 2** - Undo system
3. **Phase 3** - Power-ups
4. **Phase 4** - Game modes (Time Attack)
5. **Phase 5** - Themes and visual polish
6. **Phase 6** - Statistics and achievements
