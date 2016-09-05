#version 330

in vec3 inPoint;
in vec2 inCoord;

out vec2 coord;

void main()
{
	coord = inCoord;
	gl_Position = vec4(inPoint, 1.0);
}
