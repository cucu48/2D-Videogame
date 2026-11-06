#ifndef _TEXT_INCLUDE
#define _TEXT_INCLUDE


#include <ft2build.h>
#include FT_FREETYPE_H
#include "Texture.h"
#include "ShaderProgram.h"
#include "TexturedQuad.h"


#define NUM_CHARACTERS (128 - 32)


struct CharMetrics
{
	int tx, ty;
	int sx, sy;
	int ax, ay;
	int bl, bt;
};



class Text
{

public:
	Text();
	~Text();
	
	bool init(const char *filename);
	void destroy();
	
	ShaderProgram &getProgram();
	int getSize() const;
	int charAdvance(char c, int size) const;
	int charHeight(char c, int size) const;
	int charAscent(char c, int size) const;
	void setProjection(const glm::mat4 &projection);
	void render(char c, const glm::vec2 &pixel, int size, const glm::vec4 &color);
	void render(const string &str, const glm::vec2 &pixel, int size, const glm::vec4 &color);
	
private:
	void initShaders();
	bool extractCharSizes(int *maxCharWidth, int *maxCharHeight);
	void createTextureAtlas();
	
private:
	int fontSize, textureSize, maxCharWidth, maxCharHeight;
	FT_Face face;
	CharMetrics chars[NUM_CHARACTERS];
	Texture textureAtlas;
	ShaderProgram program;
	TexturedQuad *quad;
	glm::mat4 projection;

	static bool bLibInit;
	static FT_Library library;
	
};


#endif


