#version 330

in vec3 inPoint;
in vec2 inCoords;

out vec2 coord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void main()
{
	coord = inCoords;
	
	gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(inPoint, 1.0);
}
