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
#define DEFAULT_BLAST_RANGE 1
#define LEVEL_TIME 240000 // 4 minutes
#define RESPAWN_TIME 2000
#define VIEW_W 320
#define VIEW_H 288
#define CLEAR_TIME 2500     // ms showing "WORLD CLEAR!!" before the next level
#define GAME_OVER_TIME 3000 // ms showing "GAME OVER" before going back to the menu
#define TRENT_TYPE 9          // en el fichero de enemigos, el tipo 9 es el jefe Trent
#define BUBBLE_SPEED 1.5f
#define SPORE_SPEED 2.f
#define BOMB_RAIN_RANGE 4
#define BOMB_RAIN_FUSE 2000


Level::Level(int number)
{
	this->number = number;
	map = NULL;
	player = NULL;
	boss = NULL;
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
	for(size_t i = 0; i < projectiles.size(); i++)
		delete projectiles[i];
	projectiles.clear();
	if(boss != NULL)
		delete boss;
	boss = NULL;
}

void Level::init()
{
	bombSheet.loadFromFile("images/bomb.png", TEXTURE_PIXEL_FORMAT_RGBA);
	bombSheet.setMinFilter(GL_NEAREST);
	bombSheet.setMagFilter(GL_NEAREST);
	explosionSheet.loadFromFile("images/explosion.png", TEXTURE_PIXEL_FORMAT_RGBA);
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
	initialEnemies = int(enemies.size()) + (boss != NULL ? 1 : 0);
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
			explodeBomb(i);
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

	updateEnemies(deltaTime);
	updateBoss(deltaTime);
	updateProjectiles(deltaTime);

	// Damage to player
	if(!player->isDead() && !player->isInvulnerable() && isOnFire(player->hitboxPos(), player->hitboxSize()))
	{
		hurtPlayer();
	}
	// Se acaba el tiempo: el jugador muere (el corazon no le salva)
	if(timeLeft == 0 && !player->isDead() && !Game::instance().isGodMode())
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
	return enemies.empty() && boss == NULL;
}

bool Level::touchesPlayer(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	if(player->isDead() || player->isInvulnerable())
		return false;
	glm::ivec2 pp = player->hitboxPos(), ps = player->hitboxSize();
	return pp.x < pos.x + size.x && pp.x + ps.x > pos.x && pp.y < pos.y + size.y && pp.y + ps.y > pos.y;
}

// Enemigos: se mueven, mueren con el fuego y hacen dano al jugador si le tocan
void Level::updateEnemies(int deltaTime)
{
	glm::ivec2 playerCenter = player->hitboxPos() + player->hitboxSize() / 2;
	for(size_t i = 0; i < enemies.size(); )
	{
		Enemy *enemy = enemies[i];
		enemy->update(deltaTime, playerCenter, !player->isDead());
		if(!enemy->isDying() && isOnFire(enemy->hitboxPos(), enemy->hitboxSize()))
			enemy->kill();
		if(!enemy->isDying() && touchesPlayer(enemy->hitboxPos(), enemy->hitboxSize()))
			hurtPlayer();

		// rayo de Elekin Man
		glm::ivec2 boxPos, boxSize;
		if(enemy->attackBox(boxPos, boxSize) && touchesPlayer(boxPos, boxSize))
			hurtPlayer();

		// burbujas de Cloudy
		glm::vec2 shotPos, shotDir;
		if(enemy->takeShot(shotPos, shotDir))
		{
			Projectile *bubble = new Projectile();
			bubble->init(BUBBLE, shotPos, shotDir * BUBBLE_SPEED, &enemySheet, Game::instance().getTexProgram());
			projectiles.push_back(bubble);
		}

		if(enemy->isGone())
		{
			delete enemy;
			enemies.erase(enemies.begin() + i);
		}
		else
			i++;
	}
}

// Trent: solo le hacen dano las explosiones de las bombas del jugador
void Level::updateBoss(int deltaTime)
{
	if(boss == NULL)
		return;
	glm::ivec2 playerCenter = player->hitboxPos() + player->hitboxSize() / 2;
	boss->update(deltaTime, playerCenter, !player->isDead());
	if(!boss->isDying())
	{
		if(isOnFire(boss->hitboxPos(), boss->hitboxSize(), true) && boss->hit() && boss->isDying())
		{
			// al morir se lleva con el sus hojas y sus proyectiles
			for(size_t i = 0; i < enemies.size(); i++)
				enemies[i]->kill();
			for(size_t i = 0; i < projectiles.size(); i++)
				projectiles[i]->pop();
		}
		else if(touchesPlayer(boss->hitboxPos(), boss->hitboxSize()))
			hurtPlayer();
		bossAttack(boss->takeAttack());
	}
	if(boss->isGone())
	{
		delete boss;
		boss = NULL;
	}
}

void Level::bossAttack(TrentAttack attack)
{
	if(attack == TRENT_NO_ATTACK)
		return;
	ShaderProgram &texProgram = Game::instance().getTexProgram();
	int tileSize = map->getTileSize();
	glm::ivec2 pos = boss->getPosition();
	glm::vec2 bottomCenter(pos.x + 32.f, pos.y + 64.f);

	if(attack == TRENT_LEAVES)
	{
		// tres hojas que persiguen al jugador
		const glm::ivec2 offsets[3] = { glm::ivec2(-32, 16), glm::ivec2(16, 40), glm::ivec2(64, 16) };
		for(int i = 0; i < 3; i++)
			addEnemy(LEAF, pos + offsets[i], (i == 0) ? -1 : 1);
	}
	else if(attack == TRENT_POLLEN)
	{
		// tres esporas hacia abajo: la del medio recta y las otras en diagonal
		const glm::vec2 dirs[3] = { glm::vec2(-0.6f, 0.8f), glm::vec2(0.f, 1.f), glm::vec2(0.6f, 0.8f) };
		for(int i = 0; i < 3; i++)
		{
			Projectile *spore = new Projectile();
			spore->init(SPORE, bottomCenter, dirs[i] * SPORE_SPEED, &enemySheet, texProgram);
			projectiles.push_back(spore);
		}
	}
	else if(attack == TRENT_BOMB_RAIN)
	{
		// cuatro bombas en forma de T: tres en fila y una debajo de la del medio
		const glm::vec2 offsets[4] = { glm::vec2(-2.f * tileSize, 0.f), glm::vec2(0.f, 0.f),
		                               glm::vec2(2.f * tileSize, 0.f), glm::vec2(0.f, float(tileSize)) };
		for(int i = 0; i < 4; i++)
		{
			glm::vec2 center = bottomCenter + glm::vec2(0.f, tileSize / 2.f) + offsets[i];
			// si sale dentro de una pared, la baja hasta un hueco libre
			for(int tries = 0; tries < 4 && map->collisionBox(glm::ivec2(center) - glm::ivec2(12, 12), glm::ivec2(24, 28)); tries++)
				center.y += tileSize;
			Projectile *bomb = new Projectile();
			bomb->init(FALLING_BOMB, center, glm::vec2(0.f, 0.f), &bombSheet, texProgram);
			projectiles.push_back(bomb);
		}
	}
}

// Burbujas, esporas y bombas que caen
void Level::updateProjectiles(int deltaTime)
{
	int tileSize = map->getTileSize();
	for(size_t i = 0; i < projectiles.size(); )
	{
		Projectile *p = projectiles[i];
		p->update(deltaTime, map);
		if(p->isHarmful() && touchesPlayer(p->hitboxPos(), p->hitboxSize()))
		{
			hurtPlayer();
			p->pop();
		}

		// la bomba que cae se convierte en una bomba de verdad al tocar el suelo
		if(p->getKind() == FALLING_BOMB && p->hasLanded())
		{
			glm::ivec2 center = p->hitboxPos() + p->hitboxSize() / 2;
			glm::ivec2 tile(center.x / tileSize, center.y / tileSize);
			bool occupied = map->isSolid(tile.x, tile.y);
			for(size_t b = 0; b < bombs.size(); b++)
				if(bombs[b]->getTile() == tile)
					occupied = true;
			if(!occupied)
			{
				Bomb *bomb = new Bomb();
				bomb->init(tile, tileSize, glm::ivec2(0, 0), &bombSheet, Game::instance().getTexProgram());
				bomb->setEnemyBomb(BOMB_RAIN_RANGE, true, BOMB_RAIN_FUSE);
				bombs.push_back(bomb);
			}
			delete p;
			projectiles.erase(projectiles.begin() + i);
			continue;
		}

		if(p->isFinished())
		{
			delete p;
			projectiles.erase(projectiles.begin() + i);
		}
		else
			i++;
	}
}

void Level::addEnemy(int type, const glm::ivec2 &pos, int dir)
{
	Enemy *enemy = new Enemy();
	enemy->init(type, glm::ivec2(0, 0), &enemySheet, Game::instance().getTexProgram());
	enemy->setTileMap(map);
	enemy->setPosition(pos);
	enemy->setDirection(dir);
	enemies.push_back(enemy);
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
// tipo = 0-8 (ver Enemy.h) o 9 para el jefe Trent, direccion = -1 izquierda / 1 derecha.
// La posicion es el tile donde estan los pies del enemigo (Trent ocupa 2x2 tiles
// hacia abajo y a la derecha desde ahi).
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
		if(!(ss >> type >> col >> row >> dir) || type < 0 || type > TRENT_TYPE)
			continue;
		int tileSize = map->getTileSize();
		if(type == TRENT_TYPE)
		{
			if(boss != NULL)
				continue;
			boss = new Trent();
			boss->init(glm::ivec2(0, 0), &enemySheet, Game::instance().getTexProgram());
			boss->setTileMap(map);
			boss->setPosition(glm::ivec2(col, row) * tileSize);
			continue;
		}
		// los enemigos altos (Elekin Man) se colocan con los pies en el tile indicado
		int height = (type == ELEKIN) ? 2 * tileSize : tileSize;
		addEnemy(type, glm::ivec2(col * tileSize, (row + 1) * tileSize - height), dir);
	}
}
//TODO Debug only. Remove later.
void Level::keyPressed(int key)
{
	if(key == GLFW_KEY_H)
		player->setHeart(!player->hasHeart());
	if(key == GLFW_KEY_B)
		player->toggleDance();
	// G: modo dios, el jugador no puede morir
	if(key == GLFW_KEY_G)
	{
		Game::instance().toggleGodMode();
		cout << "Modo dios: " << (Game::instance().isGodMode() ? "activado" : "desactivado") << endl;
	}
	// K: mata a todos los enemigos del nivel (y al jefe) para abrir la puerta
	if(key == GLFW_KEY_K && state == LEVEL_PLAYING)
	{
		for(size_t i = 0; i < enemies.size(); i++)
			enemies[i]->kill();
		if(boss != NULL)
			boss->kill();
		for(size_t i = 0; i < projectiles.size(); i++)
			projectiles[i]->pop();
	}
}

// The heart absorbs one hit (the player blinks, immune, like after respawning). Without it the player dies.
void Level::hurtPlayer()
{
	if(Game::instance().isGodMode())
		return;
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
	int playerBombs = 0;
	for(size_t i = 0; i < bombs.size(); i++)
		if(!bombs[i]->isEnemyBomb())
			playerBombs++;
	if(playerBombs >= maxBombs)
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

// La explosion llega 'range' tiles en cada direccion (las bombas de Trent solo
// en horizontal). Una pared corta el brazo y otra bomba en el camino explota tambien.
void Level::explodeBomb(int index)
{
	Bomb *bomb = bombs[index];
	glm::ivec2 tile = bomb->getTile();
	int range = bomb->isEnemyBomb() ? bomb->getRange() : blastRange;
	bool horizontalOnly = bomb->isHorizontalOnly();
	bool fromEnemy = bomb->isEnemyBomb();
	map->setBlocked(tile.x, tile.y, false);
	delete bomb;
	bombs.erase(bombs.begin() + index);

	addExplosion(tile, EXPLOSION_CENTER, fromEnemy);
	const glm::ivec2 dirs[4] = { glm::ivec2(0, -1), glm::ivec2(0, 1), glm::ivec2(-1, 0), glm::ivec2(1, 0) };
	const ExplosionPiece ends[4] = { EXPLOSION_UP, EXPLOSION_DOWN, EXPLOSION_LEFT, EXPLOSION_RIGHT };
	for(int d = horizontalOnly ? 2 : 0; d < 4; d++)
	{
		ExplosionPiece mid = (d < 2) ? EXPLOSION_VMID : EXPLOSION_HMID;
		for(int i = 1; i <= range; i++)
		{
			glm::ivec2 t = tile + dirs[d] * i;
			if(!addExplosion(t, (i == range) ? ends[d] : mid, fromEnemy))
				break;
			bool hitBomb = false;
			for(size_t b = 0; b < bombs.size(); b++)
				if(bombs[b]->getTile() == t)
				{
					bombs[b]->detonate(); // explota en el siguiente frame
					hitBomb = true;
				}
			if(hitBomb)
				break;
		}
	}
}

bool Level::addExplosion(const glm::ivec2 &tile, ExplosionPiece piece, bool fromEnemy)
{
	if(piece != EXPLOSION_CENTER && map->isWall(tile.x, tile.y))
		return false;

	Explosion *explosion = new Explosion();
	explosion->init(tile, piece, map->getTileSize(), glm::ivec2(0, 0), &explosionSheet, Game::instance().getTexProgram());
	explosion->setFromEnemy(fromEnemy);
	explosions.push_back(explosion);
	return true;
}

bool Level::isOnFire(const glm::ivec2 &pos, const glm::ivec2 &size, bool onlyPlayerFire) const
{
	for(size_t i = 0; i < explosions.size(); i++)
		if(!(onlyPlayerFire && explosions[i]->isFromEnemy()) && overlapsTile(pos, size, explosions[i]->getTile()))
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
	if(boss != NULL)
		boss->render();
	for(size_t i = 0; i < enemies.size(); i++)
		enemies[i]->render();
	for(size_t i = 0; i < projectiles.size(); i++)
		projectiles[i]->render();
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
	hudState.enemiesLeft = int(enemies.size()) + (boss != NULL ? 1 : 0);
	hudState.maxBombs = maxBombs;
	hudState.blastRange = blastRange;
	hud.render(hudState);
}

