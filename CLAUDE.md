# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

"Pocket Bomberman": a Game Boy-style Bomberman clone in C++11 with OpenGL 3 (GLEW, GLFW, GLM, SOIL). Code comments are a mix of English and Spanish; match whichever the surrounding file uses. There are no tests and no linter.

## Commands

- Install deps (Debian/Ubuntu): `make install-debian`
- Build: `make` (outputs `build/game`; flags `-std=c++11 -Wall -Wextra -Wpedantic -O0 -g`; every `*.cpp` in the root is picked up by wildcard)
- Run: `make run`. **Run from the repo root**, because assets are loaded with relative paths (`images/...`, `levels/...`, `shaders/...`).
- Clean: `make clean`

## Architecture

**Main loop (`main.cpp`).** The window is 640x480. Game logic advances in fixed 1/60 s steps using an accumulator (capped at 0.25 s), and `update(deltaTime)` always receives 16 ms. Rendering and vsync happen once per frame. GLFW callbacks forward input to the `Game` singleton, and cursor positions are rescaled from window to framebuffer pixels.

**Game singleton (`Game.h/.cpp`).**
- It owns a `vector<Scene*>` with a fixed layout: index 0 is the menu (`SCENE_MENU`), 1 is the level selector (`SCENE_LEVEL_SELECT`), and 2 onward are levels (`SCENE_FIRST_LEVEL`). The number of levels is `NUM_LEVELS` (5).
- Adding a level means bumping `NUM_LEVELS` and adding `levels/Stage1-<n>.txt`.
- It forwards update, render and input to the current scene. `changeScene()` calls the target's `onEnter()`, so scenes reset their state there.
- It owns the single shared `texProgram` shader (`shaders/texture.*`), and everything draws through it via `Game::instance().getTexProgram()`.
- Number keys 1-9 and keypad 1-9 jump straight to that level from any scene and are not forwarded to the scene. ESC quits from anywhere.
- Held-key state is polled with `Game::getKey()`. `Scene::keyPressed` only receives press edges.

**Scenes (`Scene.h`).** This is an abstract interface with `init` (called once, creates GL resources), `onEnter`, `update`, `render(windowW, windowH)` and optional input hooks. `MenuScene`, `LevelSelectScene` and `Level` implement it. Each scene's `render` letterboxes into its own virtual resolution through the `Scene::setLetterboxViewport` and `windowToVirtual` helpers. Menu screens use 160x144 virtual pixels. Levels use the map size in pixels.

**Level (`Level.cpp`).** This is the gameplay scene, and it orchestrates the entities:
- A `TileMap`, a `Player`, and vectors of `Enemy`, `Bomb` and `Explosion`.
- `Level::loadLevel` and `cleanup` are re-run on every `onEnter`, so each entry starts the stage fresh.
- Collision between entities is done here, using hitbox AABB overlap against explosion tiles (`isOnFire`).
- Bombs start passable. `Level::update` turns one solid with `TileMap::setBlocked` once the player's hitbox leaves its tile, and unblocks the tile when it explodes.
- Explosions are one tile in each of 4 directions, and walls (`TileMap::isWall`) stop them.
- `MAX_BOMBS` is 1. Z places a bomb, and the player respawns after `RESPAWN_TIME`.

**Entities.** `Player` and `Enemy` use a `Sprite`, which animates from `AnimKeyframes` over a spritesheet `Texture`. Both are platformer-style: gravity, jumping, and collision against `TileMap` through `collisionMove*` / `collisionBox`. `Enemy` has three behaviours (WALKER, JUMPER, FLYER) selected by `type`, which is the row of `images/enemies.png` (types 0-6).

**TileMap.** It builds a single VBO for the whole map. It tracks static solidity (from the level file's `NONSOLID` list) separately from dynamic `blockedTile` (bombs).

## Data formats

- **Level file `levels/Stage1-<n>.txt`:**
  - Line 1 is `TILEMAP`.
  - Then come map size `W H`, `tileSize blockSize`, the tilesheet path, the tilesheet grid `cols rows`, and the blank tile code.
  - Next is `NONSOLID id id ...`. Every tile id not listed is solid, and `TileMap::isWall` and `isSolid` build on this.
  - The rest is `H` rows of comma-separated tile ids.
  - Parsing is strict. A malformed file makes `createTileMap` throw `runtime_error`.
- **Enemy file `levels/Stage1-<n>_enemies.txt`** (optional; a missing file means no enemies): one enemy per line as `type col row direction` (direction is -1 or 1). Lines starting with `#` are comments.
- **`scripts/level_converter.cpp`** is a standalone tool, not part of the Makefile build. Usage is `./level_converter input.png output.png`. It splits a screenshot into 16x16 tiles, dedupes them into a power-of-two tilesheet PNG, and prints the level file's header and tile grid to stdout. It needs `stb_image.h` and `stb_image_write.h`, which are not in the repo, so you have to supply them yourself. The compiled binary is gitignored. `images/Stage1-N.png` and `Stage1-N_TileSheet.png` are its inputs and outputs. `levels/level01.txt` is a leftover that nothing loads.

## Notes

- `plans/SceneImplementationPlan.md` is an untracked design doc for the scene system. It is already implemented, so treat it as background only.
