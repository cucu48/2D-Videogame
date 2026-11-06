#version 330

uniform mat4 projection, modelview;

in vec2 position;

void main()
{
	gl_Position = projection * modelview * vec4(position, 0.0, 1.0);
}
