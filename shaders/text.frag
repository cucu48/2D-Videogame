#version 330

uniform vec4 color;
uniform sampler2D tex;

in vec2 texCoordFrag;
out vec4 outColor;

void main()
{
	outColor = vec4(color.rgb, color.a * texture(tex, texCoordFrag).r);
}
