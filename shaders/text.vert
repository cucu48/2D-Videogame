#version 330

uniform mat4 projection, modelview;
uniform vec2 minTexCoord, maxTexCoord;

in vec2 position;
in vec2 texCoord;
out vec2 texCoordFrag;

void main()
{
	texCoordFrag = mix(minTexCoord, maxTexCoord, texCoord);
	gl_Position = projection * modelview * vec4(position, 0.0, 1.0);
}
