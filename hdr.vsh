#version 420

layout (location = 0) in vec3 vertexPosition;

// matrix transforms
uniform mat4 model, view, projection;

void main()
{
	gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
}