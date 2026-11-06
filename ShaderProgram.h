#ifndef _SHADER_PROGRAM_INCLUDE
#define _SHADER_PROGRAM_INCLUDE


#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include "Shader.h"



class ShaderProgram
{

public:
	ShaderProgram();

	void init();
	void addShader(const Shader &shader);
	void bindFragmentOutput(const string &outputName);
	GLint bindVertexAttribute(const string &attribName, GLint size, GLsizei stride, GLvoid *firstPointer);
	void link();
	void free();

	void use();

	void setUniform2f(const string &uniformName, float v0, float v1);
	void setUniform3f(const string &uniformName, float v0, float v1, float v2);
	void setUniform4f(const string &uniformName, float v0, float v1, float v2, float v3);
	void setUniformMatrix4f(const string &uniformName, glm::mat4 &mat);

	bool isLinked();
	const string &log() const;

private:
	GLuint programId;
	bool linked;
	string errorLog;

};


#endif
