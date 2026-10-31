#version 330

uniform mat4 projection, modelview;
uniform vec2 minTexCoord, maxTexCoord;

in vec2 position;
in vec2 texCoord;
out vec2 texCoordFrag;

void main()
{
	// The quad covers (0,0)-(1,1) and its texCoords go 0..1: pick the glyph inside the atlas
	texCoordFrag = mix(minTexCoord, maxTexCoord, texCoord);
	gl_Position = projection * modelview * vec4(position, 0.0, 1.0);
}
