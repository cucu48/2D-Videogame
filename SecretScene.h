#ifndef _SECRET_SCENE_INCLUDE
#define _SECRET_SCENE_INCLUDE


#include "Scene.h"
#include "Texture.h"
#include "TexturedQuad.h"
#include "Sprite.h"
#include "DitherFade.h"
#include "Text.h"


// Final secreto: si se derrota al jefe con los 4 bongs, aparece un quinto en el
// centro del mapa y al cogerlo se funde a esta escena. Un prado al atardecer
// con Bomberman y Eyti sentados en la hierba, frente a frente, fumando. El fondo y los efectos
// salen de scripts/make_secret_scene.py; aqui se anima el bong que comparten
// (burbujas y brasa al dar la calada) y las bocanadas de humo.
// La musica (MUSIC_SECRET) la pone el nivel durante el fundido, al tercer
// segundo, y sigue sonando aqui. Espacio o enter vuelven al menu.
// Tambien hace de creditos (en catalan): salen de detras de las colinas y
// suben por el cielo (se pintan entre secret_bg.png y secret_fg.png).


class SecretScene : public Scene
{

public:
	SecretScene();
	~SecretScene();

	void init();
	void onEnter();
	void update(int deltaTime);
	void render(int windowWidth, int windowHeight);
	void keyPressed(int key);

private:
	void renderCell(TexturedQuad *quad, const glm::vec2 &pos, float scale);
	void renderCredits(const glm::mat4 &projection);

private:
	Texture bgTex, fgTex, fxTex, promptTex;
	TexturedQuad *bgQuad, *fgQuad, *promptQuad;
	Text text;
	bool bText;                      // la fuente se ha cargado bien
	TexturedQuad *bongQuads[2][3];   // [normal, espejo][reposo, calada A, calada B]
	TexturedQuad *smokeQuads[4];     // de pequena a grande
	DitherFade fade;
	int time;
	int leaveTime;                   // -1 mientras no se sale; si no, ms desde que se pulso
	bool bInScene;

};


#endif // _SECRET_SCENE_INCLUDE
