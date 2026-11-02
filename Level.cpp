#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Level.h"
#include "Game.h"
#include <algorithm>
#include <fstream>
#include <sstream>



#define INIT_PLAYER_X_TILES 4
#define DEFAULT_MAX_BOMBS 5
#define DEFAULT_BLAST_RANGE 3
#define LEVEL_TIME 240000 // 4 minutes
#define RESPAWN_TIME 2000
#define VIEW_W 320
#define VIEW_H 288
#define CLEAR_TIME 2500     // ms showing "WORLD CLEAR!!" before the next level
#define GAME_OVER_TIME 3000 // ms showing "GAME OVER" before going back to the menu


Level::Level(int number)
{
	this->number = number;
	map = NULL;
	player = NULL;
	clearSprite = NULL;
	gameOverSprite = NULL;
}

Level::~Level()
{
	cleanup();
	if(clearSprite != NULL)
		delete clearSprite;
	if(gameOverSprite != NULL)
		delete gameOverSprite;
}

// Frees everything created by loadLevel
void Level::cleanup()
{
	if(map != NULL)
		delete map;
	map = NULL;
	if(player != NULL)
		delete player;
	player = NULL;
	for(size_t i = 0; i < bombs.size(); i++)
		delete bombs[i];
	bombs.clear();
	for(size_t i = 0; i < explosions.size(); i++)
		delete explosions[i];
	explosions.clear();
	for(size_t i = 0; i < enemies.size(); i++)
		delete enemies[i];
	enemies.clear();
}

void Level::init()
{
	bombSheet.loadFromFile("images/bomb.png", TEXTURE_PIXEL_FORMAT_RGBA);
	bombSheet.setMinFilter(GL_NEAREST);
	bombSheet.setMagFilter(GL_NEAREST);
	explosionSheet.loadFromFile("images/explosion2.png", TEXTURE_PIXEL_FORMAT_RGBA);
	explosionSheet.setMinFilter(GL_NEAREST);
	explosionSheet.setMagFilter(GL_NEAREST);
	enemySheet.loadFromFile("images/enemies.png", TEXTURE_PIXEL_FORMAT_RGBA);
	enemySheet.setMinFilter(GL_NEAREST);
	enemySheet.setMagFilter(GL_NEAREST);
	hud.init();

	// Carteles de fin de nivel (imagenes a x2)
	ShaderProgram &texProgram = Game::instance().getTexProgram();
	clearTex.loadFromFile("images/WorldClear.png", TEXTURE_PIXEL_FORMAT_RGBA);
	clearTex.setMinFilter(GL_NEAREST);
	clearTex.setMagFilter(GL_NEAREST);
	clearSprite = Sprite::createSprite(glm::vec2(clearTex.width() * 2, clearTex.height() * 2), glm::vec2(1.f, 1.f), &clearTex, &texProgram);
	gameOverTex.loadFromFile("images/GameOver.png", TEXTURE_PIXEL_FORMAT_RGBA);
	gameOverTex.setMinFilter(GL_NEAREST);
	gameOverTex.setMagFilter(GL_NEAREST);
	gameOverSprite = Sprite::createSprite(glm::vec2(gameOverTex.width() * 2, gameOverTex.height() * 2), glm::vec2(1.f, 1.f), &gameOverTex, &texProgram);

	loadLevel();
}

// Every time the level is entered it starts from scratch
void Level::onEnter()
{
	cleanup();
	loadLevel();
}

void Level::loadLevel()
{
	ShaderProgram &texProgram = Game::instance().getTexProgram();
	string stage = "levels/Stage1-" + to_string(number);
	map = TileMap::createTileMap(stage + ".txt", glm::vec2(0, 0), texProgram);
	player = new Player();
	player->init(glm::ivec2(0, 0), texProgram);
	spawnPos = glm::vec2(map->findSpawn(INIT_PLAYER_X_TILES, glm::ivec2(32, 32)));
	player->setPosition(spawnPos);
	player->setTileMap(map);
	bHasDoor = false;
	loadEnemies(stage + "_enemies.txt");
	initialEnemies = int(enemies.size());
	state = LEVEL_PLAYING;
	stateTime = 0;
	deathTime = 0;
	bombKeyWasPressed = false;
	updateCamera();
	currentTime = 0.0f;
	timeLeft = LEVEL_TIME;
	maxBombs = DEFAULT_MAX_BOMBS;
	blastRange = DEFAULT_BLAST_RANGE;
}

void Level::update(int deltaTime)
{
	currentTime += deltaTime;

	// Fin del nivel: solo se espera a que pase el cartel
	if(state != LEVEL_PLAYING)
	{
		stateTime += deltaTime;
		if(state == LEVEL_CLEARED && stateTime >= CLEAR_TIME)
			finishLevel();
		else if(state == LEVEL_GAME_OVER && stateTime >= GAME_OVER_TIME)
			Game::instance().changeScene(SCENE_MENU);
		return;
	}

	timeLeft = std::max(0, timeLeft - deltaTime);
	player->update(deltaTime);

	// Z: place bomb
	bool bombKey = Game::instance().getKey(GLFW_KEY_F);
	if(bombKey && !bombKeyWasPressed && !player->isDead())
		placeBomb();
	bombKeyWasPressed = bombKey;

	// Bombas: cuenta atras y paso de "atravesable" a "solida"
	for(size_t i = 0; i < bombs.size(); i++)
	{
		bombs[i]->update(deltaTime);
		if(bombs[i]->isPassable() && !overlapsTile(player->hitboxPos(), player->hitboxSize(), bombs[i]->getTile()))
		{
			bombs[i]->makeSolid();
			map->setBlocked(bombs[i]->getTile().x, bombs[i]->getTile().y, true);
		}
	}
	for(size_t i = 0; i < bombs.size(); )
	{
		if(bombs[i]->hasToExplode())
		{
			explodeBomb(i);
			i = 0; // chain reactions may have flagged bombs before this index
		}
		else
			i++;
	}

	// Explosions
	for(size_t i = 0; i < explosions.size(); )
	{
		explosions[i]->update(deltaTime);
		if(explosions[i]->isFinished())
		{
			delete explosions[i];
			explosions.erase(explosions.begin() + i);
		}
		else
			i++;
	}

	// Enemigos: se mueven, mueren con el fuego y matan al jugador si le tocan
	for(size_t i = 0; i < enemies.size(); )
	{
		Enemy *enemy = enemies[i];
		enemy->update(deltaTime);
		if(!enemy->isDying() && isOnFire(enemy->hitboxPos(), enemy->hitboxSize()))
			enemy->kill();
		if(!enemy->isDying() && !player->isDead() && !player->isInvulnerable())
		{
			glm::ivec2 pp = player->hitboxPos(), ps = player->hitboxSize();
			glm::ivec2 ep = enemy->hitboxPos(), es = enemy->hitboxSize();
			if(pp.x < ep.x + es.x && pp.x + ps.x > ep.x && pp.y < ep.y + es.y && pp.y + ps.y > ep.y)
			{
				hurtPlayer();
			}
		}
		if(enemy->isGone())
		{
			delete enemy;
			enemies.erase(enemies.begin() + i);
		}
		else
			i++;
	}

	// Damage to player
	if(!player->isDead() && !player->isInvulnerable() && isOnFire(player->hitboxPos(), player->hitboxSize()))
	{
		hurtPlayer();
	}
	// Se acaba el tiempo: el jugador muere (el corazon no le salva)
	if(timeLeft == 0 && !player->isDead())
	{
		player->kill();
		deathTime = 0;
		Game::instance().loseLife();
	}

	if(player->isDead())
	{
		deathTime += deltaTime;
		if(deathTime >= RESPAWN_TIME)
		{
			if(Game::instance().getLives() == 0)
			{
				state = LEVEL_GAME_OVER;
				stateTime = 0;
			}
			else
			{
				player->respawn(spawnPos);
				if(timeLeft == 0)
					timeLeft = LEVEL_TIME;
			}
		}
	}

	// Ganar: con todos los enemigos eliminados, llegar a la puerta
	else if(isDoorOpen() && (bHasDoor ? playerAtDoor() : initialEnemies > 0))
	{
		player->stopDance();
		state = LEVEL_CLEARED;
		stateTime = 0;
	}

	updateCamera();
}

// La puerta se abre cuando no queda ningun enemigo (ni siquiera desapareciendo)
bool Level::isDoorOpen() const
{
	return enemies.empty();
}

// El centro del jugador esta dentro del tile de la puerta
bool Level::playerAtDoor() const
{
	int tileSize = map->getTileSize();
	glm::ivec2 center = player->hitboxPos() + player->hitboxSize() / 2;
	return center.x / tileSize == doorTile.x && center.y / tileSize == doorTile.y;
}

void Level::finishLevel()
{
	if(number < NUM_LEVELS)
		Game::instance().startLevel(number + 1);
	else
		Game::instance().changeScene(SCENE_MENU);
}

// Centra la camara en el jugador sin salirse del mapa. Si el mapa es mas
// pequeno que la vista en un eje, lo centra en ese eje.
void Level::updateCamera()
{
	glm::vec2 mapPx = glm::vec2(map->getMapSize()) * float(map->getTileSize());
	glm::vec2 viewPx(VIEW_W, VIEW_H);
	glm::vec2 target = glm::vec2(player->hitboxPos()) + glm::vec2(player->hitboxSize()) / 2.f;

	for(int axis = 0; axis < 2; axis++)
	{
		float range = mapPx[axis] - viewPx[axis];
		if(range <= 0.f)
			camera[axis] = range / 2.f;
		else
			camera[axis] = glm::clamp(target[axis] - viewPx[axis] / 2.f, 0.f, range);
		camera[axis] = std::floor(camera[axis]);
	}
}

// Lee los enemigos del nivel. Formato (una linea por enemigo):
//   tipo columna fila direccion
// tipo = fila de images/enemies.png (0-6), direccion = -1 izquierda / 1 derecha.
// Las lineas que empiezan por # son comentarios.
// Una linea "DOOR columna fila" indica el tile de la puerta de salida. Si un
// nivel no tiene puerta, se supera al eliminar a todos los enemigos.
void Level::loadEnemies(const string &enemiesFile)
{
	ifstream fin(enemiesFile.c_str());
	if(!fin.is_open())
		return;
	string line;
	while(getline(fin, line))
	{
		if(line.empty() || line[0] == '#')
			continue;
		stringstream ss(line);
		if(line.compare(0, 4, "DOOR") == 0)
		{
			ss.ignore(4);
			if(ss >> doorTile.x >> doorTile.y)
				bHasDoor = true;
			continue;
		}
		int type, col, row, dir;
		if(!(ss >> type >> col >> row >> dir) || type < 0 || type > 6)
			continue;
		Enemy *enemy = new Enemy();
		enemy->init(type, glm::ivec2(0, 0), &enemySheet, Game::instance().getTexProgram());
		enemy->setTileMap(map);
		enemy->setPosition(glm::ivec2(col, row) * map->getTileSize());
		enemy->setDirection(dir);
		enemies.push_back(enemy);
	}
}
//TODO Debug only. Remove later.
void Level::keyPressed(int key)
{
	if(key == GLFW_KEY_H)
		player->setHeart(!player->hasHeart());
	if(key == GLFW_KEY_B)
		player->toggleDance();
}

// The heart absorbs one hit (the player blinks, immune, like after respawning). Without it the player dies.
void Level::hurtPlayer()
{
	if(player->hasHeart())
	{
		player->setHeart(false);
		player->startInvulnerable();
		return;
	}
	player->kill();
	deathTime = 0;
	Game::instance().loseLife();
}

void Level::placeBomb()
{
	if(int(bombs.size()) >= maxBombs)
		return;

	// tile donde esta el centro del jugador
	int tileSize = map->getTileSize();
	glm::ivec2 center = player->hitboxPos() + player->hitboxSize() / 2;
	glm::ivec2 tile(center.x / tileSize, center.y / tileSize);
	if(map->isSolid(tile.x, tile.y))
		return; // ya hay una bomba (o una pared) en ese tile

	Bomb *bomb = new Bomb();
	bomb->init(tile, tileSize, glm::ivec2(0, 0), &bombSheet, Game::instance().getTexProgram());
	bombs.push_back(bomb);
}

void Level::explodeBomb(int index)
{
	glm::ivec2 tile = bombs[index]->getTile();
	int skipArm = bombs[index]->getSkipArm();
	map->setBlocked(tile.x, tile.y, false);
	delete bombs[index];
	bombs.erase(bombs.begin() + index);

	addExplosion(tile, EXPLOSION_CENTER);

	const glm::ivec2 dirs[4] = { glm::ivec2(0, -1), glm::ivec2(0, 1), glm::ivec2(-1, 0), glm::ivec2(1, 0) };
	const ExplosionPiece tips[4] = { EXPLOSION_UP, EXPLOSION_DOWN, EXPLOSION_LEFT, EXPLOSION_RIGHT };
	const ExplosionPiece middles[4] = { EXPLOSION_VERTICAL, EXPLOSION_VERTICAL, EXPLOSION_HORIZONTAL, EXPLOSION_HORIZONTAL };

	for(int d = 0; d < 4; d++)
	{
		if(d == skipArm)
			continue; // this arm points back at the bomb that triggered us
		for(int step = 1; step <= blastRange; step++)
		{
			glm::ivec2 t = tile + dirs[d] * step;
			if(map->isWall(t.x, t.y))
				break; // walls stop the arm, no flames on them
			int hit = findBomb(t);
			if(hit >= 0)
			{
				bombs[hit]->detonate(d ^ 1); // chain reaction; the arm stops here like on a wall
				break;
			}
			// tip only when the arm reaches its full length; a wall-shortened arm keeps the arm piece
			addExplosion(t, step == blastRange ? tips[d] : middles[d]);
		}
	}
}

int Level::findBomb(const glm::ivec2 &tile) const
{
	for(size_t i = 0; i < bombs.size(); i++)
		if(bombs[i]->getTile() == tile)
			return int(i);
	return -1;
}

void Level::addExplosion(const glm::ivec2 &tile, ExplosionPiece piece)
{
	Explosion *explosion = new Explosion();
	explosion->init(tile, piece, map->getTileSize(), glm::ivec2(0, 0), &explosionSheet, Game::instance().getTexProgram());
	explosions.push_back(explosion);

}

bool Level::isOnFire(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	for(size_t i = 0; i < explosions.size(); i++)
		if(overlapsTile(pos, size, explosions[i]->getTile()))
			return true;
	return false;
}

bool Level::overlapsTile(const glm::ivec2 &pos, const glm::ivec2 &size, const glm::ivec2 &tile) const
{
	int tileSize = map->getTileSize();
	int tx = tile.x * tileSize, ty = tile.y * tileSize;
	return pos.x < tx + tileSize && pos.x + size.x > tx &&
	       pos.y < ty + tileSize && pos.y + size.y > ty;
}

void Level::render(int windowWidth, int windowHeight)
{
	// The virtual screen is the playfield with the HUD strip below it: split the letterboxed viewport
	setLetterboxViewport(windowWidth, windowHeight, VIEW_W, VIEW_H + HUD_HEIGHT);
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	int hudPx = int(vp[3] * float(HUD_HEIGHT) / (VIEW_H + HUD_HEIGHT) + 0.5f);
	glViewport(vp[0], vp[1] + hudPx, vp[2], vp[3] - hudPx);
	projection = glm::ortho(camera.x, camera.x + VIEW_W, camera.y + VIEW_H, camera.y);

	glm::mat4 modelview;

	ShaderProgram &texProgram = Game::instance().getTexProgram();
	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	modelview = glm::mat4(1.0f);
	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	map->render();
	for(size_t i = 0; i < bombs.size(); i++)
		bombs[i]->render();
	for(size_t i = 0; i < enemies.size(); i++)
		enemies[i]->render();
	for(size_t i = 0; i < explosions.size(); i++)
		explosions[i]->render();
	player->render();

	// Cartel de fin de nivel, centrado en la vista
	Sprite *banner = (state == LEVEL_CLEARED) ? clearSprite : (state == LEVEL_GAME_OVER) ? gameOverSprite : NULL;
	if(banner != NULL)
	{
		Texture &tex = (state == LEVEL_CLEARED) ? clearTex : gameOverTex;
		glm::vec2 size(tex.width() * 2, tex.height() * 2);
		banner->setPosition(glm::floor(camera + (glm::vec2(VIEW_W, VIEW_H) - size) / 2.f));
		banner->render();
	}

	glViewport(vp[0], vp[1], vp[2], hudPx);
	HudState hudState;
	hudState.secondsLeft = (timeLeft + 999) / 1000;
	hudState.hasHeart = player->hasHeart();
	hudState.lives = Game::instance().getLives();
	hudState.enemiesLeft = int(enemies.size());
	hudState.maxBombs = maxBombs;
	hudState.blastRange = blastRange;
	hud.render(hudState);
}

