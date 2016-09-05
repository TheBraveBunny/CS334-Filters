#version 330

in vec2 coord;

uniform sampler2D textureData;

void main()
{
	gl_FragColor = texture(textureData, coord);
}
