#version 330

uniform vec4 color;
uniform sampler2D tex;

in vec2 texCoordFrag;
out vec4 outColor;

void main()
{
	// The atlas only has one channel (GL_RED) holding the glyph coverage
	outColor = vec4(color.rgb, color.a * texture(tex, texCoordFrag).r);
}
