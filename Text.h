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
	int tx, ty; 		// Pos x, y inside texture in pixels
	int sx, sy; 		// Size of char in pixels
	int ax, ay; 		// Advance for each character in a string sequence
	int bl, bt; 		// Bitmap left and top adjusting
};


// Using Freetype the Text class is capable of building a texture atlas
// for a given font and size. Combining with TexturedQuad allows it 
// to render text strings


class Text
{

public:
	Text();
	~Text();
	
	bool init(const char *filename);
	void destroy();
	
	ShaderProgram &getProgram();
	int getSize() const;
	int charAdvance(char c, int size) const;   // advance in pixels of c drawn at the given size
	int charHeight(char c, int size) const;    // height in pixels of the bitmap of c drawn at the given size
	int charAscent(char c, int size) const;    // pixels from the baseline up to the top of the bitmap of c
	void setProjection(const glm::mat4 &projection);   // pixel space the pixel arguments of render refer to
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


#endif // _TEXT_INCLUDE


