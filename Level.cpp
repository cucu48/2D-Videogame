#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Level.h"
#include "Game.h"
#include "Audio.h"
#include "Sounds.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>



#define INIT_PLAYER_X_TILES 4
#define LEVEL_TIME 240000 // 4 minutes
#define RESPAWN_TIME 2000
#define VIEW_W 320
#define VIEW_H 288
#define SECRET_FADE_TIME 4000  // ms desde que se coge el quinto bong hasta el final secreto
#define SECRET_DARK_TIME 3000  // ms del fundido a negro (luego se queda en negro)
#define SECRET_MUSIC_TIME 3000 // ms en que empieza a sonar MUSIC_SECRET
#define FINAL_BONG_TILE_X 7.5f // el quinto bong cae en el centro de la plataforma del jefe
#define FINAL_BONG_TILE_Y 5
#define STAGE_CARD_TIME 1500 // ms del cartel "STAGE 1-N" al empezar un stage
#define HURRY_TIME 30000    // ms: con menos tiempo la musica se acelera
#define POWERUP_FLASH_TIME 450
#define LEAF_BITS 7         // trocitos de hoja por arbusto
#define LEAF_LIFE 700       // ms
#define CLEAR_TIME 5500     // ms showing "WORLD CLEAR!!" before the next level (lo que dura music_clear)
#define GAME_OVER_TIME 3000 // ms showing "GAME OVER" before going back to the menu
#define BUBBLE_SPEED 1.5f
#define SPORE_SPEED 2.f
#define BOMB_RAIN_RANGE 4
#define BOMB_RAIN_FUSE 2000
#define WATERFALL_STEP 180    // ms entre cada tile de agua que se retira
#define MAX_BOMBS_LIMIT 9     // Bomb Up
#define MAX_BLAST_RANGE 4     // Fire Up / Full Fire
#define ARMOR_TIME 10000      // ms de inmunidad de la chaqueta
#define FREEZE_TIME 20000     // ms que dura el reloj (timer)


Level::Level(int number)
{
	this->number = number;
	map = NULL;
	player = NULL;
	boss = NULL;
	door = NULL;
	clearSprite = NULL;
	gameOverSprite = NULL;
	bongSprite = NULL;
	for(int i = 0; i < 4; i++)
		leafQuads[i] = NULL;
}

Level::~Level()
{
	cleanup();
	if(clearSprite != NULL)
		delete clearSprite;
	if(gameOverSprite != NULL)
		delete gameOverSprite;
	if(bongSprite != NULL)
		delete bongSprite;
	for(int i = 0; i < 4; i++)
		if(leafQuads[i] != NULL)
		{
			leafQuads[i]->free();
			delete leafQuads[i];
		}
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
	for(size_t i = 0; i < pickups.size(); i++)
		delete pickups[i];
	pickups.clear();
	itemUnderBush.clear();
	if(door != NULL)
		delete door;
	door = NULL;
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
	powerUpSheet.loadFromFile("images/PowerUps.png", TEXTURE_PIXEL_FORMAT_RGBA);
	powerUpSheet.setMinFilter(GL_NEAREST);
	powerUpSheet.setMagFilter(GL_NEAREST);
	doorSheet.loadFromFile("images/Doors.png", TEXTURE_PIXEL_FORMAT_RGBA);
	doorSheet.setMinFilter(GL_NEAREST);
	doorSheet.setMagFilter(GL_NEAREST);
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
	vsScreen.init(texProgram);
	fade.init(texProgram, VIEW_W, VIEW_H + HUD_HEIGHT);
	pauseMenu.init();
	instructions.init();
	leafTex.loadFromFile("images/leaf_bits.png", TEXTURE_PIXEL_FORMAT_RGBA);
	leafTex.setMinFilter(GL_NEAREST);
	leafTex.setMagFilter(GL_NEAREST);
	for(int i = 0; i < 4; i++)
	{
		glm::vec2 geom[2] = { glm::vec2(0.f), glm::vec2(8.f) };
		glm::vec2 texCoords[2] = { glm::vec2(i / 4.f, 0.f), glm::vec2((i + 1) / 4.f, 1.f) };
		leafQuads[i] = TexturedQuad::createTexturedQuad(geom, texCoords, texProgram);
	}
	bPaused = false;
	pauseScreen = PAUSE_MAIN;

	// Coleccionable: images/bong.png, 2 frames de 16x16 (el segundo con brillo) a x2
	bongTex.loadFromFile("images/bong.png", TEXTURE_PIXEL_FORMAT_RGBA);
	bongTex.setMinFilter(GL_NEAREST);
	bongTex.setMagFilter(GL_NEAREST);
	bongSprite = Sprite::createSprite(glm::vec2(32.f, 32.f), glm::vec2(0.5f, 1.f), &bongTex, &texProgram);
	bongSprite->setNumberAnimations(1);
	bongSprite->setAnimationSpeed(0, 2);
	bongSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	bongSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	bongSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
	bongSprite->addKeyframe(0, glm::vec2(0.5f, 0.f));
	bongSprite->changeAnimation(0);

	loadLevel();
}

// Every time the level is entered it starts from scratch
void Level::onEnter()
{
	cleanup();
	bPaused = false;
	Audio::instance().setMusicDucked(false);
	loadLevel();
}

// Vuelta desde el menu principal: todo sigue como estaba, solo vuelve la musica
void Level::onResume()
{
	bPaused = false;
	Audio::instance().setMusicDucked(false);
	if(state == LEVEL_SECRET_FADE && stateTime >= SECRET_MUSIC_TIME)
		Audio::instance().playMusic(MUSIC_SECRET, true);
	else if(!player->isDead())
		Audio::instance().playMusic(levelMusic(), true);
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
	// Los power-ups permanentes pasan de un nivel al siguiente (ver Game::powerUps)
	PowerUpState &carried = Game::instance().powerUps();
	player->setSpeedMul(carried.speedMul);
	player->setWings(carried.wings);
	player->setHeart(carried.heart);
	waterfalls.clear();
	bBongHere = false;
	bongPopupTime = 0;
	bFinalBong = false;
	loadEntities(stage + "_entities.txt");
	initialEnemies = int(enemies.size()) + (boss != NULL ? 1 : 0);
	state = LEVEL_PLAYING;
	stateTime = 0;
	deathTime = 0;
	bombKeyWasPressed = false;
	updateCamera(true);
	currentTime = 0.0f;
	timeLeft = LEVEL_TIME;
	maxBombs = carried.maxBombs;
	blastRange = carried.blastRange;
	bFullFire = false;
	remoteEnabled = carried.remote;
	freezeTime = 0;
	bBossLevel = (boss != NULL);
	powerUpFlash = 0;
	leafBits.clear();
	hud.reset();
	Audio::instance().setMusicHurry(false);
	Audio::instance().playMusic(levelMusic(), true);
	if(bBossLevel)
	{
		state = LEVEL_INTRO;
		vsScreen.start();
	}
	else
		state = LEVEL_STAGE_CARD;   // "STAGE 1-N" antes de empezar
}

std::string Level::levelMusic() const
{
	if(bBossLevel)
		return MUSIC_BOSS;
	return MUSIC_STAGE + to_string((number - 1) % NUM_STAGE_MUSICS + 1);
}

void Level::update(int deltaTime)
{
	if(bPaused)
		return;
	currentTime += deltaTime;

	// Pantalla VS del jefe: el nivel esta parado hasta que acaba
	if(state == LEVEL_INTRO)
	{
		vsScreen.update(deltaTime);
		if(!vsScreen.isActive())
			state = LEVEL_PLAYING;
		return;
	}
	if(state == LEVEL_STAGE_CARD)
	{
		stateTime += deltaTime;
		if(stateTime >= STAGE_CARD_TIME)
		{
			state = LEVEL_PLAYING;
			stateTime = 0;
		}
		return;
	}

	// Fin del nivel: solo se espera a que pase el cartel
	if(state != LEVEL_PLAYING)
	{
		Audio::instance().setMusicHurry(false);
		stateTime += deltaTime;
		if(state == LEVEL_CLEARED && stateTime >= CLEAR_TIME)
			finishLevel();
		else if(state == LEVEL_GAME_OVER && stateTime >= GAME_OVER_TIME)
			Game::instance().changeScene(SCENE_MENU);
		else if(state == LEVEL_SECRET_FADE)
		{
			if(stateTime >= SECRET_MUSIC_TIME && stateTime - deltaTime < SECRET_MUSIC_TIME)
				Audio::instance().playMusic(MUSIC_SECRET, true);
			if(stateTime >= SECRET_FADE_TIME)
				Game::instance().changeScene(SCENE_SECRET);
		}
		return;
	}

	// Timer: mientras dura, el reloj y todo lo enemigo se detienen
	player->setGodMode(Game::instance().isGodMode());
	bool frozen = freezeTime > 0;
	if(frozen)
		freezeTime = std::max(0, freezeTime - deltaTime);
	else
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
		if(!(frozen && bombs[i]->isEnemyBomb()))
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

	updateWaterfalls(deltaTime);
	updateEnemies(deltaTime, frozen);
	updateBoss(deltaTime, frozen);
	updateProjectiles(deltaTime, frozen);
	updatePickups();
	updateBong(deltaTime);
	updateFinalBong(deltaTime);
	updateLeafBits(deltaTime);
	if(powerUpFlash > 0)
		powerUpFlash -= deltaTime;
	hud.update(deltaTime, hudState());
	// poco tiempo: la musica se acelera (no con la de morir ni la del baile)
	Audio::instance().setMusicHurry(timeLeft <= HURRY_TIME && !player->isDead() && !player->isDancing());
	if(state != LEVEL_PLAYING)
		return;

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
		Audio::instance().playMusic(MUSIC_DEATH, false);
	}

	// Sonido al abrirse la puerta (ya no quedan enemigos)
	// La puerta se abre (y deja de ser solida) cuando ya no quedan enemigos
	if(door != NULL && !door->isOpen() && isDoorOpen())
	{
		door->open();
		Audio::instance().playSound(SFX_DOOR_OPEN);
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
				Audio::instance().playMusic(MUSIC_GAME_OVER, false);
			}
			else
			{
				player->respawn(spawnPos);
				Audio::instance().playMusic(levelMusic(), true);
				if(timeLeft == 0)
					timeLeft = LEVEL_TIME;
			}
		}
	}

	// Ganar: con todos los enemigos eliminados, llegar a la puerta
	// Jefe derrotado con los 4 bongs: en vez de acabar, cae el quinto en el centro
	else if(bBossLevel && Game::instance().bongCount() == NUM_BONGS && isDoorOpen())
	{
		if(!bFinalBong)
		{
			bFinalBong = true;
			finalBongPos = glm::vec2(FINAL_BONG_TILE_X * map->getTileSize() - 16.f, camera.y - 40.f);
			finalBongVel = 0.f;
			Audio::instance().playSound(SFX_DOOR_OPEN);
		}
	}
	else if(isDoorOpen() && (door != NULL ? playerAtDoor() : initialEnemies > 0))
	{
		player->stopDance();
		state = LEVEL_CLEARED;
		stateTime = 0;
		Audio::instance().playMusic(MUSIC_CLEAR, false);
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
void Level::updateEnemies(int deltaTime, bool frozen)
{
	glm::ivec2 playerCenter = player->hitboxPos() + player->hitboxSize() / 2;
	for(size_t i = 0; i < enemies.size(); )
	{
		Enemy *enemy = enemies[i];
		if(!frozen || enemy->isDying())
			enemy->update(deltaTime, playerCenter, !player->isDead());
		if(!enemy->isDying() && isOnFire(enemy->hitboxPos(), enemy->hitboxSize()))
		{
			enemy->kill();
			Audio::instance().playSound(SFX_ENEMY_DIE);
		}
		if(!enemy->isDying() && touchesPlayer(enemy->hitboxPos(), enemy->hitboxSize()))
			hurtPlayer();

		// rayo de Elekin Man
		glm::ivec2 boxPos, boxSize;
		if(!frozen && enemy->attackBox(boxPos, boxSize) && touchesPlayer(boxPos, boxSize))
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
void Level::updateBoss(int deltaTime, bool frozen)
{
	if(boss == NULL)
		return;
	glm::ivec2 playerCenter = player->hitboxPos() + player->hitboxSize() / 2;
	if(!frozen || boss->isDying())
		boss->update(deltaTime, playerCenter, !player->isDead());
	if(!boss->isDying())
	{
		if(isOnFire(boss->hitboxPos(), boss->hitboxSize(), true) && boss->hit())
		{
			Audio::instance().playSound(SFX_BOSS_HIT);
			// al morir se lleva con el sus hojas y sus proyectiles
			if(boss->isDying())
			{
				for(size_t i = 0; i < enemies.size(); i++)
					enemies[i]->kill();
				for(size_t i = 0; i < projectiles.size(); i++)
					projectiles[i]->pop();
			}
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
void Level::updateProjectiles(int deltaTime, bool frozen)
{
	int tileSize = map->getTileSize();
	for(size_t i = 0; i < projectiles.size(); )
	{
		Projectile *p = projectiles[i];
		if(!frozen || p->isPopping())
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
	return door != NULL && center.x / tileSize == door->getTile().x && center.y / tileSize == door->getTile().y;
}

void Level::finishLevel()
{
	// Los power-ups permanentes pasan al siguiente nivel (armor y timer no)
	PowerUpState &carried = Game::instance().powerUps();
	carried.maxBombs = maxBombs;
	carried.blastRange = blastRange;
	carried.remote = remoteEnabled;
	carried.speedMul = player->getSpeedMul();
	carried.wings = player->hasRealWings();
	carried.heart = player->hasHeart();
	if(number < NUM_LEVELS)
		Game::instance().startLevel(number + 1);
	else
		Game::instance().changeScene(SCENE_CREDITS);   // juego acabado (sin el final secreto)
}

// Camara con zona muerta: el jugador se mueve libremente dentro de un
// rectangulo centrado en la pantalla sin que la camara se mueva; al tocar su
// borde la camara lo sigue. Siempre se mantiene dentro del mapa (si el mapa es
// mas pequeno que la vista en un eje, se centra en ese eje).
// snap = true centra la camara en el jugador (inicio de nivel).
void Level::updateCamera(bool snap)
{
	const glm::vec2 deadHalf(VIEW_W / 6.f, VIEW_H / 6.f); // zona muerta = 1/3 de la vista
	glm::vec2 mapPx = glm::vec2(map->getMapSize()) * float(map->getTileSize());
	glm::vec2 viewPx(VIEW_W, VIEW_H);
	glm::vec2 target = glm::vec2(player->hitboxPos()) + glm::vec2(player->hitboxSize()) / 2.f;

	for(int axis = 0; axis < 2; axis++)
	{
		float range = mapPx[axis] - viewPx[axis];
		if(range <= 0.f)
		{
			camera[axis] = std::floor(range / 2.f);
			continue;
		}
		float cam = camera[axis];
		if(snap)
			cam = target[axis] - viewPx[axis] / 2.f;
		else
		{
			float center = cam + viewPx[axis] / 2.f;
			if(target[axis] < center - deadHalf[axis])
				cam = target[axis] + deadHalf[axis] - viewPx[axis] / 2.f;
			else if(target[axis] > center + deadHalf[axis])
				cam = target[axis] - deadHalf[axis] - viewPx[axis] / 2.f;
		}
		camera[axis] = std::floor(glm::clamp(cam, 0.f, range));
	}
}

// Lee las entidades del nivel (levels/Stage1-N_entities.txt). Una por linea:
//   kind col row [args...]
// Las lineas vacias y las que empiezan por # se ignoran. Un fichero que falta
// significa que no hay entidades. Un error lanza runtime_error con fichero y linea.
//   enemy col row type dir     type 0-8 (ver Enemy.h), dir -1 izquierda / 1 derecha.
//                              La posicion es el tile donde estan los pies (los
//                              enemigos altos, Elekin Man, ocupan hacia arriba)
//   boss col row               el jefe Trent (2x2 tiles hacia abajo y a la derecha)
//   door col row               puerta de salida (maximo una). Sin puerta, el nivel
//                              se supera al eliminar a todos los enemigos
//   waterfall col headRow bottomRow   la cabeza esta en (col, headRow) y el agua
//                              llega hasta bottomRow
//   item col row type          power-up escondido bajo el arbusto de ese tile
//                              (bombup fireup speedup remote armor fullfire heart timer wings)
void Level::loadEntities(const string &entitiesFile)
{
	ifstream fin(entitiesFile.c_str());
	if(!fin.is_open())
		return;
	int tileSize = map->getTileSize();
	glm::ivec2 mapSize = map->getMapSize();
	string line;
	int lineNumber = 0;
	while(getline(fin, line))
	{
		lineNumber++;
		size_t comment = line.find('#');
		if(comment != string::npos)
			line.erase(comment);
		stringstream ss(line);
		string kind;
		if(!(ss >> kind))
			continue;
		string where = entitiesFile + ":" + to_string(lineNumber) + ": ";
		int col, row;
		if(!(ss >> col >> row))
			throw runtime_error(where + "expected 'col row' after '" + kind + "'");
		if(col < 0 || col >= mapSize.x || row < 0 || row >= mapSize.y)
			throw runtime_error(where + "tile (" + to_string(col) + ", " + to_string(row) + ") is outside the map");

		if(kind == "enemy")
		{
			int type, dir;
			if(!(ss >> type >> dir) || type < 0 || type >= NUM_ENEMY_TYPES || (dir != -1 && dir != 1))
				throw runtime_error(where + "enemy needs 'type dir' (type 0-" + to_string(NUM_ENEMY_TYPES - 1) + ", dir -1 or 1)");
			// los enemigos altos (Elekin Man) se colocan con los pies en el tile indicado
			int height = (type == ELEKIN) ? 2 * tileSize : tileSize;
			addEnemy(type, glm::ivec2(col * tileSize, (row + 1) * tileSize - height), dir);
		}
		else if(kind == "boss")
		{
			if(boss != NULL)
				throw runtime_error(where + "more than one boss");
			boss = new Trent();
			boss->init(glm::ivec2(0, 0), &enemySheet, Game::instance().getTexProgram());
			boss->setTileMap(map);
			boss->setPosition(glm::ivec2(col, row) * tileSize);
		}
		else if(kind == "door")
		{
			if(door != NULL)
				throw runtime_error(where + "more than one door");
			door = new Door();
			door->init(glm::ivec2(col, row), map, &doorSheet, Game::instance().getTexProgram());
		}
		else if(kind == "waterfall")
		{
			Waterfall w;
			w.head = glm::ivec2(col, row);
			if(!(ss >> w.bottomRow) || w.bottomRow < row || w.bottomRow >= mapSize.y)
				throw runtime_error(where + "waterfall needs 'col headRow bottomRow' with bottomRow >= headRow, inside the map");
			w.nextRow = w.bottomRow;
			w.endTile = map->getTile(col, w.bottomRow);
			w.timer = 0;
			w.bRaising = false;
			waterfalls.push_back(w);
		}
		else if(kind == "bong")
		{
			bongTile = glm::ivec2(col, row);
			bBongHere = !Game::instance().hasBong(number); // si ya se cogio, no vuelve a salir
		}
		else if(kind == "item")
		{
			string name;
			PowerUpType type;
			if(!(ss >> name) || !parsePowerUp(name, &type))
				throw runtime_error(where + "item needs a type: bombup fireup speedup remote armor fullfire heart timer wings");
			if(!map->isBush(col, row))
			{
				cerr << where << "warning: no bush at (" << col << ", " << row << "), item ignored" << endl;
				continue;
			}
			itemUnderBush[row * mapSize.x + col] = type;
		}
		else
			throw runtime_error(where + "unknown entity '" + kind + "'");
	}
}

//TODO Debug only. Remove later.
void Level::keyPressed(int key)
{
	// Pausa: las teclas son para sus botones
	if(bPaused)
	{
		if(key == GLFW_KEY_ESCAPE)
		{
			Audio::instance().playSound(SFX_SELECT);
			if(pauseScreen != PAUSE_MAIN)
				showPauseScreen(PAUSE_MAIN);
			else
				closePause();
			return;
		}
		bool onVolume = pauseScreen == PAUSE_OPTIONS && pauseMenu.getSelected() == 0;
		if(onVolume && (key == GLFW_KEY_LEFT || key == GLFW_KEY_A))
			changeVolume(-1);
		else if(onVolume && (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D))
			changeVolume(1);
		else
			pauseChoose(pauseMenu.keyPressed(key));
		return;
	}
	if(key == GLFW_KEY_ESCAPE)
	{
		if(state == LEVEL_PLAYING)
			openPause();
		return;
	}
	// Pantalla VS: espacio o enter la saltan; el resto de teclas no hace nada
	if(state == LEVEL_INTRO)
	{
		if(key == GLFW_KEY_SPACE || key == GLFW_KEY_ENTER)
			vsScreen.skip();
		return;
	}
	// Cartel "STAGE 1-N": espacio o enter lo saltan
	if(state == LEVEL_STAGE_CARD)
	{
		if(key == GLFW_KEY_SPACE || key == GLFW_KEY_ENTER)
		{
			state = LEVEL_PLAYING;
			stateTime = 0;
		}
		return;
	}
	if(state == LEVEL_SECRET_FADE)
		return;
	// D: con el mando a distancia, detona la bomba del jugador mas antigua
	if(key == GLFW_KEY_D && (remoteEnabled || Game::instance().isGodMode()) && state == LEVEL_PLAYING && !player->isDead())
	{
		for(size_t i = 0; i < bombs.size(); i++)
			if(!bombs[i]->isEnemyBomb())
			{
				bombs[i]->detonate();
				break;
			}
	}
	// E: truco de todos los bongs (lo aplica Game); el de este nivel ya no sale
	if(key == GLFW_KEY_E)
		bBongHere = false;
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
		if(!enemies.empty() || boss != NULL)
			Audio::instance().playSound(SFX_ENEMY_DIE);
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
		Audio::instance().playSound(SFX_HURT);
		player->startInvulnerable();
		return;
	}
	player->kill();
	deathTime = 0;
	Game::instance().loseLife();
	Audio::instance().playMusic(MUSIC_DEATH, false);
}

void Level::placeBomb()
{
	int playerBombs = 0;
	for(size_t i = 0; i < bombs.size(); i++)
		if(!bombs[i]->isEnemyBomb())
			playerBombs++;
	if(playerBombs >= (Game::instance().isGodMode() ? MAX_BOMBS_LIMIT : maxBombs))
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
	Audio::instance().playSound(SFX_BOMB_PLACE);
}

// La explosion llega 'range' tiles en cada direccion (las bombas de Trent solo
// en horizontal). Una pared corta el brazo y otra bomba en el camino explota tambien.
void Level::explodeBomb(int index)
{
	Bomb *bomb = bombs[index];
	glm::ivec2 tile = bomb->getTile();
	int range = bomb->isEnemyBomb() ? bomb->getRange() : currentBlastRange();
	bool horizontalOnly = bomb->isHorizontalOnly();
	bool fromEnemy = bomb->isEnemyBomb();
	int skipArm = bomb->getSkipArm();
	map->setBlocked(tile.x, tile.y, false);
	delete bomb;
	bombs.erase(bombs.begin() + index);

	addExplosion(tile, EXPLOSION_CENTER, fromEnemy);
	Audio::instance().playSound(SFX_EXPLOSION);
	const glm::ivec2 dirs[4] = { glm::ivec2(0, -1), glm::ivec2(0, 1), glm::ivec2(-1, 0), glm::ivec2(1, 0) };
	const ExplosionPiece ends[4] = { EXPLOSION_UP, EXPLOSION_DOWN, EXPLOSION_LEFT, EXPLOSION_RIGHT };
	for(int d = horizontalOnly ? 2 : 0; d < 4; d++)
	{
		if(d == skipArm)
			continue; // this arm points back at the bomb that triggered us
		ExplosionPiece mid = (d < 2) ? EXPLOSION_VMID : EXPLOSION_HMID;
		for(int i = 1; i <= range; i++)
		{
			glm::ivec2 t = tile + dirs[d] * i;
			if(map->isBush(t.x, t.y))
			{
				destroyBush(t); // ahora es cielo, asi que se puede poner fuego encima
				addExplosion(t, ends[d], fromEnemy);
				break; // el arbusto corta el brazo
			}
			if(isClosedDoorTile(t))
				break; // la puerta cerrada es rigida: el fuego no la pasa
			if(map->isWall(t.x, t.y))
			{
				hitWaterfall(t); // la cabeza de una cascada se levanta
				break; // walls stop the arm, no flames on them
			}
			int hit = findBomb(t);
			if(hit >= 0)
			{
				bombs[hit]->detonate(d ^ 1); // chain reaction; the arm stops here like on a wall
				break;
			}
			addExplosion(t, (i == range) ? ends[d] : mid, fromEnemy);
		}
	}
}

bool Level::isClosedDoorTile(const glm::ivec2 &tile) const
{
	return door != NULL && !door->isOpen() && door->getTile() == tile;
}

// El arbusto pasa a ser cielo y, si escondia un power-up, aparece en su tile
void Level::destroyBush(const glm::ivec2 &tile)
{
	map->setTile(tile.x, tile.y, map->getBlankTile());
	spawnLeafBits(tile);
	std::map<int, PowerUpType>::iterator it = itemUnderBush.find(tile.y * map->getMapSize().x + tile.x);
	if(it == itemUnderBush.end())
		return;
	Pickup *pickup = new Pickup();
	pickup->init(it->second, tile, map->getTileSize(), &powerUpSheet, Game::instance().getTexProgram());
	pickups.push_back(pickup);
	itemUnderBush.erase(it);
}

// El jugador recoge los power-ups que toca
void Level::updatePickups()
{
	if(player->isDead())
		return;
	glm::ivec2 pp = player->hitboxPos(), ps = player->hitboxSize();
	for(size_t i = 0; i < pickups.size(); )
	{
		glm::ivec2 ip = pickups[i]->hitboxPos(), is = pickups[i]->hitboxSize();
		if(pp.x < ip.x + is.x && pp.x + ps.x > ip.x && pp.y < ip.y + is.y && pp.y + ps.y > ip.y)
		{
			applyPowerUp(pickups[i]->getType());
			Audio::instance().playSound(SFX_POWERUP);
			powerUpFlash = POWERUP_FLASH_TIME;
			delete pickups[i];
			pickups.erase(pickups.begin() + i);
		}
		else
			i++;
	}
}

// Coleccionable: se coge al tocarlo y sale un aviso con los que llevas
#define BONG_POPUP_TIME 2000

void Level::updateBong(int deltaTime)
{
	if(bongPopupTime > 0)
		bongPopupTime -= deltaTime;
	if(!bBongHere || player->isDead())
		return;
	if(overlapsTile(player->hitboxPos(), player->hitboxSize(), bongTile))
	{
		bBongHere = false;
		Game::instance().collectBong(number);
		bongPopupTime = BONG_POPUP_TIME;
		Audio::instance().playSound(SFX_COLLECT);
	}
}

// Quinto bong: cae con gravedad, rebota un poco y al cogerlo se funde a negro
void Level::updateFinalBong(int deltaTime)
{
	if(!bFinalBong)
		return;
	float groundY = float(FINAL_BONG_TILE_Y * map->getTileSize());
	finalBongVel += 0.0008f * deltaTime;   // px/ms^2
	finalBongPos.y += finalBongVel * deltaTime;
	if(finalBongPos.y >= groundY)
	{
		finalBongPos.y = groundY;
		finalBongVel = (finalBongVel > 0.15f) ? -finalBongVel * 0.35f : 0.f;
	}
	if(player->isDead())
		return;
	glm::ivec2 pp = player->hitboxPos(), ps = player->hitboxSize();
	glm::vec2 bp = finalBongPos + glm::vec2(6.f, 2.f), bs(20.f, 30.f);
	if(pp.x < bp.x + bs.x && pp.x + ps.x > bp.x && pp.y < bp.y + bs.y && pp.y + ps.y > bp.y)
	{
		bFinalBong = false;
		player->stopDance();
		state = LEVEL_SECRET_FADE;
		stateTime = 0;
		Audio::instance().playSound(SFX_COLLECT);
		Audio::instance().stopMusic();   // silencio hasta SECRET_MUSIC_TIME
	}
}

void Level::renderBong()
{
	ShaderProgram &program = Game::instance().getTexProgram();
	int tileSize = map->getTileSize();
	if(bBongHere)
	{
		bongSprite->update(16);
		// flota arriba y abajo
		float bob = std::floor(2.f * std::sin(currentTime * 0.004f));
		bongSprite->setPosition(glm::vec2(bongTile * tileSize) + glm::vec2(0.f, bob));
		bongSprite->render();
	}
	if(bFinalBong)
	{
		// brilla (el shader multiplica el color: >1 lo aclara)
		float glow = 1.5f + 0.5f * std::sin(currentTime * 0.012f);
		program.setUniform4f("color", glow, glow, glow * 0.7f, 1.f);
		bongSprite->setPosition(glm::floor(finalBongPos));
		bongSprite->render();
		program.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	}
	if(bongPopupTime > 0)
	{
		// los 4 bongs encima del jugador: los cogidos en color, los que faltan en sombra
		float t = 1.f - float(bongPopupTime) / BONG_POPUP_TIME;
		float rise = std::min(1.f, t * 6.f) * 16.f;
		glm::vec2 center = glm::vec2(player->hitboxPos()) + glm::vec2(player->hitboxSize().x / 2.f, 0.f);
		float width = NUM_BONGS * 32.f + (NUM_BONGS - 1) * 2.f;
		glm::vec2 pos = glm::floor(glm::vec2(center.x - width / 2.f, center.y - 40.f - rise));
		pos.x = std::max(camera.x + 2.f, std::min(pos.x, camera.x + VIEW_W - width - 2.f));
		pos.y = std::max(camera.y + 2.f, pos.y);
		for(int i = 0; i < NUM_BONGS; i++)
		{
			bool got = Game::instance().hasBong(i + 1);
			// el que se acaba de coger parpadea
			if(i + 1 == number && (bongPopupTime / 120) % 2 == 0 && t < 0.5f)
				program.setUniform4f("color", 2.f, 2.f, 2.f, 1.f);
			else if(got)
				program.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
			else
				program.setUniform4f("color", 0.15f, 0.15f, 0.2f, 1.f);
			bongSprite->setPosition(pos + glm::vec2(i * 34.f, 0.f));
			bongSprite->render();
		}
		program.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	}
}

int Level::currentBlastRange() const
{
	return (bFullFire || Game::instance().isGodMode()) ? MAX_BLAST_RANGE : blastRange;
}

void Level::applyPowerUp(PowerUpType type)
{
	switch(type)
	{
	case PU_BOMB_UP:
		maxBombs = std::min(maxBombs + 1, MAX_BOMBS_LIMIT);
		break;
	case PU_FIRE_UP:
		// tambien cuenta con el Full Fire activo: se nota en el stage siguiente
		blastRange = std::min(blastRange + 1, MAX_BLAST_RANGE);
		break;
	case PU_FULL_FIRE:
		bFullFire = true;   // solo este stage (finishLevel pasa blastRange, no el maximo)
		break;
	case PU_SPEED_UP:
		player->addSpeed();
		break;
	case PU_REMOTE:
		remoteEnabled = true;
		break;
	case PU_ARMOR:
		player->startInvulnerable(ARMOR_TIME);
		break;
	case PU_HEART:
		if(!player->hasHeart()) // no se acumulan
			player->setHeart(true);
		break;
	case PU_TIMER:
		freezeTime = FREEZE_TIME;
		break;
	case PU_WINGS:
		player->setWings(true);
		break;
	}
}

int Level::findBomb(const glm::ivec2 &tile) const
{
	for(size_t i = 0; i < bombs.size(); i++)
		if(bombs[i]->getTile() == tile)
			return int(i);
	return -1;
}

void Level::hitWaterfall(const glm::ivec2 &tile)
{
	for(size_t i = 0; i < waterfalls.size(); i++)
		if(waterfalls[i].head == tile && !waterfalls[i].bRaising)
		{
			waterfalls[i].bRaising = true;
			Audio::instance().playSound(SFX_WATERFALL);
			waterfalls[i].timer = 0;
		}
}

// El agua se retira de abajo arriba: cada WATERFALL_STEP ms la fila mas baja
// pasa a ser cielo (no solido) y el tile final de la cascada se pone una fila
// mas arriba. Se para cuando solo queda un tile de agua bajo la cabeza.
void Level::updateWaterfalls(int deltaTime)
{
	for(size_t i = 0; i < waterfalls.size(); i++)
	{
		Waterfall &w = waterfalls[i];
		if(!w.bRaising || w.nextRow <= w.head.y + 1)
			continue;
		w.timer -= deltaTime;
		if(w.timer > 0)
			continue;
		map->setTile(w.head.x, w.nextRow, map->getBlankTile());
		w.nextRow--;
		map->setTile(w.head.x, w.nextRow, w.endTile);
		w.timer = WATERFALL_STEP;
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

HudState Level::hudState() const
{
	HudState s;
	s.secondsLeft = (timeLeft + 999) / 1000;
	s.hasHeart = player->hasHeart();
	s.lives = Game::instance().getLives();
	s.enemiesLeft = int(enemies.size()) + (boss != NULL ? 1 : 0);
	s.maxBombs = Game::instance().isGodMode() ? MAX_BOMBS_LIMIT : maxBombs;
	s.blastRange = currentBlastRange();
	return s;
}

// Cartel al empezar: la partida se ve oscurecida y "STAGE 1-N" en grande
void Level::renderStageCard()
{
	ShaderProgram &program = Game::instance().getTexProgram();
	fade.render(program, 4);
	glm::mat4 projection = glm::ortho(0.f, float(VIEW_W), float(VIEW_H + HUD_HEIGHT), 0.f);
	pauseMenu.renderLabel(projection, "STAGE 1-" + to_string(number), VIEW_W / 2.f, 150.f, 24,
	                      glm::vec4(1.f, 0.85f, 0.3f, 1.f));
}

// --- Restos de hoja ---

void Level::spawnLeafBits(const glm::ivec2 &tile)
{
	int tileSize = map->getTileSize();
	glm::vec2 center = glm::vec2(tile * tileSize) + glm::vec2(tileSize / 2.f);
	for(int i = 0; i < LEAF_BITS; i++)
	{
		LeafBit b;
		float angle = 3.14159f * (0.15f + 0.7f * float(rand() % 100) / 100.f);   // hacia arriba, abiertos
		float speed = 1.5f + float(rand() % 100) / 50.f;
		b.vel = glm::vec2(std::cos(angle) * speed, -std::sin(angle) * speed);
		b.pos = center + glm::vec2(float(rand() % 17 - 8), float(rand() % 17 - 8)) - glm::vec2(4.f);
		b.life = LEAF_LIFE - rand() % 200;
		b.frame = rand() % 4;
		leafBits.push_back(b);
	}
}

void Level::updateLeafBits(int deltaTime)
{
	for(size_t i = 0; i < leafBits.size(); )
	{
		LeafBit &b = leafBits[i];
		b.vel.y = std::min(b.vel.y + 0.18f, 3.f);   // caen con gravedad...
		b.vel.x *= 0.96f;                           // ...y frenan de lado, como hojas
		b.pos += b.vel;
		b.life -= deltaTime;
		// van girando (cambian de dibujo) mientras caen
		if(b.life % 120 < deltaTime)
			b.frame = (b.frame + 1) % 4;
		if(b.life <= 0)
			leafBits.erase(leafBits.begin() + i);
		else
			i++;
	}
}

void Level::renderLeafBits()
{
	ShaderProgram &program = Game::instance().getTexProgram();
	for(size_t i = 0; i < leafBits.size(); i++)
	{
		// los ultimos ms parpadean antes de desaparecer
		if(leafBits[i].life < 200 && (leafBits[i].life / 50) % 2 == 0)
			continue;
		glm::mat4 m = glm::translate(glm::mat4(1.f), glm::vec3(glm::floor(leafBits[i].pos), 0.f));
		program.setUniformMatrix4f("modelview", m);
		leafQuads[leafBits[i].frame]->render(leafTex);
	}
	glm::mat4 identity(1.f);
	program.setUniformMatrix4f("modelview", identity);
}

// --- Pausa ---

#define PAUSE_DARKEN 3          // nivel de la trama (de 8) con que se oscurece la partida
#define HELP_DARKEN 7           // mas oscuro en las instrucciones, para que se lea el texto
#define VOLUME_STEP 0.1f

void Level::openPause()
{
	bPaused = true;
	Audio::instance().playSound(SFX_SELECT);
	Audio::instance().setMusicDucked(true);
	showPauseScreen(PAUSE_MAIN);
}

void Level::closePause()
{
	bPaused = false;
	Audio::instance().setMusicDucked(false);
}

void Level::showPauseScreen(PauseScreen screen)
{
	// al volver de opciones o instrucciones, su boton sigue seleccionado
	int selected = 0;
	if(screen == PAUSE_MAIN && pauseScreen == PAUSE_OPTIONS)
		selected = 1;
	else if(screen == PAUSE_MAIN && pauseScreen == PAUSE_HELP)
		selected = 2;
	pauseScreen = screen;
	std::vector<std::string> items;
	if(screen == PAUSE_OPTIONS)
	{
		items.push_back("");   // volumen: el texto lo pone changeVolume
		items.push_back("TORNAR");
	}
	else if(screen == PAUSE_HELP)
		items.push_back("TORNAR");
	else
	{
		items.push_back("CONTINUAR");
		items.push_back("OPCIONS");
		items.push_back("INSTRUCCIONS");
		items.push_back("MENU PRINCIPAL");
	}
	pauseMenu.setItems(items);
	pauseMenu.setSelected(selected);
	if(screen == PAUSE_HELP)
		pauseMenu.setLayout(VIEW_W / 2.f, VIEW_H + HUD_HEIGHT - 12.f, 12, 20.f);
	else
		pauseMenu.setLayout(VIEW_W / 2.f, 150.f, 12, 20.f);
	if(screen == PAUSE_OPTIONS)
		changeVolume(0);
}

void Level::changeVolume(int step)
{
	Audio &audio = Audio::instance();
	if(step != 0)
	{
		audio.setVolume(audio.getVolume() + step * VOLUME_STEP);
		audio.playSound(SFX_SELECT);   // para oir como suena
	}
	int percent = int(audio.getVolume() * 100.f + 0.5f);
	pauseMenu.setItem(0, "VOLUM  - " + to_string(percent) + "% +");
}

void Level::pauseChoose(int item)
{
	if(item < 0)
		return;
	Audio::instance().playSound(SFX_SELECT);
	if(pauseScreen == PAUSE_HELP)
	{
		showPauseScreen(PAUSE_MAIN);
		return;
	}
	if(pauseScreen == PAUSE_OPTIONS)
	{
		if(item == 1)
			showPauseScreen(PAUSE_MAIN);
		return;
	}
	if(item == 0)
		closePause();
	else if(item == 1)
		showPauseScreen(PAUSE_OPTIONS);
	else if(item == 2)
		showPauseScreen(PAUSE_HELP);
	else
	{
		player->stopDance();
		bPaused = false;
		Audio::instance().setMusicDucked(false);
		Game::instance().suspendToMenu();
	}
}

bool Level::pauseMousePos(glm::vec2 &p) const
{
	Game &game = Game::instance();
	return windowToVirtual(game.getMouseX(), game.getMouseY(), game.getWindowWidth(), game.getWindowHeight(),
	                       VIEW_W, VIEW_H + HUD_HEIGHT, p);
}

void Level::mouseMove(int /*x*/, int /*y*/)
{
	glm::vec2 p;
	if(bPaused && pauseMousePos(p))
		pauseMenu.hover(p);
}

void Level::mousePress(int button)
{
	glm::vec2 p;
	if(!bPaused || button != GLFW_MOUSE_BUTTON_LEFT || !pauseMousePos(p))
		return;
	int item = pauseMenu.hover(p);
	// volumen: a la izquierda del centro baja, a la derecha sube
	if(pauseScreen == PAUSE_OPTIONS && item == 0)
		changeVolume(p.x < pauseMenu.itemCenterX() ? -1 : 1);
	else
		pauseChoose(item);
}

void Level::renderPause()
{
	ShaderProgram &program = Game::instance().getTexProgram();
	glm::mat4 projection = glm::ortho(0.f, float(VIEW_W), float(VIEW_H + HUD_HEIGHT), 0.f);
	if(pauseScreen == PAUSE_HELP)
	{
		fade.render(program, HELP_DARKEN);
		instructions.render(projection, VIEW_W / 2.f, 28.f);
	}
	else
	{
		fade.render(program, PAUSE_DARKEN);
		pauseMenu.renderLabel(projection, pauseScreen == PAUSE_OPTIONS ? "OPCIONS" : "PAUSA", VIEW_W / 2.f, 116.f, 24,
		                      glm::vec4(1.f, 1.f, 1.f, 1.f));
	}
	pauseMenu.render(projection);
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
	if(door != NULL)
		door->render();
	for(size_t i = 0; i < pickups.size(); i++)
		pickups[i]->render();
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
	// tras coger un power-up el jugador destella en dorado (el shader multiplica: >1 aclara)
	bool flash = powerUpFlash > 0 && (powerUpFlash / 60) % 2 == 0;
	if(flash)
		texProgram.setUniform4f("color", 2.f, 1.7f, 0.35f, 1.f);
	player->render();
	if(flash)
		texProgram.setUniform4f("color", 1.f, 1.f, 1.f, 1.f);
	renderLeafBits();
	renderBong();

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
	hud.render(hudState());
	if(state == LEVEL_STAGE_CARD)
	{
		setLetterboxViewport(windowWidth, windowHeight, VIEW_W, VIEW_H + HUD_HEIGHT);
		renderStageCard();
	}

	// Pantalla VS encima de todo (nivel y HUD)
	if(state == LEVEL_INTRO)
	{
		setLetterboxViewport(windowWidth, windowHeight, VIEW_W, VIEW_H + HUD_HEIGHT);
		vsScreen.render(Game::instance().getTexProgram(), VIEW_W, VIEW_H + HUD_HEIGHT);
	}
	if(bPaused)
	{
		setLetterboxViewport(windowWidth, windowHeight, VIEW_W, VIEW_H + HUD_HEIGHT);
		renderPause();
	}
	// Fundido con trama hacia el final secreto (los ultimos ms ya en negro)
	if(state == LEVEL_SECRET_FADE)
	{
		setLetterboxViewport(windowWidth, windowHeight, VIEW_W, VIEW_H + HUD_HEIGHT);
		fade.render(Game::instance().getTexProgram(), stateTime * FADE_LEVELS / SECRET_DARK_TIME + 1);
	}
}

