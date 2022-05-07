#version 420


layout(location = 0) in vec3 vertexPosition;

uniform mat4 model, lightView, lightProjection;

void main()
{
	gl_Position = lightProjection * lightView * model * vec4(vertexPosition, 1.0);
}
