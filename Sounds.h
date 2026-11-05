#ifndef _SOUNDS_INCLUDE
#define _SOUNDS_INCLUDE


// Nombres de la musica y los efectos de sonido (en la carpeta sounds/).
// Van sin extension: Audio busca .wav, .mp3 o .flac. Una musica en bucle puede
// tener al lado un "<nombre>.loop" con el primer y el ultimo frame del trozo
// que se repite (la intro suena solo una vez). Si falta un archivo el
// juego sigue sin ese sonido (y lo avisa una vez por la consola).

// Musica
#define MUSIC_TITLE      "sounds/music_title"      // menu y selector de nivel (en bucle)
#define MUSIC_STAGE      "sounds/music_stage"      // + numero del stage (1-4), en bucle
#define NUM_STAGE_MUSICS 4                         // los stages siguientes repiten desde la 1
#define MUSIC_BOSS       "sounds/music_boss"       // nivel del jefe (en bucle)
#define MUSIC_CLEAR      "sounds/music_clear"      // "WORLD CLEAR!!" (una vez)
#define MUSIC_GAME_OVER  "sounds/music_game_over"  // "GAME OVER" (una vez)
#define MUSIC_DEATH      "sounds/music_death"      // el jugador muere (una vez)
#define MUSIC_SECRET     "sounds/music_secret"     // final secreto (en bucle, empieza en el segundo 9 del tema)

// Efectos
#define SFX_SELECT       "sounds/sfx_select"       // elegir en el menu / selector
#define SFX_JUMP         "sounds/sfx_jump"
#define SFX_BOMB_PLACE   "sounds/sfx_bomb_place"
#define SFX_EXPLOSION    "sounds/sfx_explosion"
#define SFX_ENEMY_DIE    "sounds/sfx_enemy_die"
#define SFX_HURT         "sounds/sfx_hurt"         // el corazon absorbe un golpe
#define SFX_BOSS_HIT     "sounds/sfx_boss_hit"
#define SFX_DOOR_OPEN    "sounds/sfx_door_open"    // ya no quedan enemigos
#define SFX_WATERFALL    "sounds/sfx_waterfall"    // la cascada se levanta
#define SFX_COLLECT      "sounds/sfx_collect"      // coger un coleccionable (bong)

// Todos los efectos, para cargarlos en memoria al arrancar (Game::init)
static const char *const ALL_SFX[] = { SFX_SELECT, SFX_JUMP, SFX_BOMB_PLACE, SFX_EXPLOSION,
	SFX_ENEMY_DIE, SFX_HURT, SFX_BOSS_HIT, SFX_DOOR_OPEN, SFX_WATERFALL, SFX_COLLECT };


#endif // _SOUNDS_INCLUDE
