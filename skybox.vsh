#version 420

layout (location = 0) in vec3 vertexPosition;

out vec3 texCoords;

uniform mat4 projection, view, model;

void main()
{
	texCoords = vertexPosition;
	gl_Position = (projection * view * model * vec4(vertexPosition, 1.f)).xyww;
}

