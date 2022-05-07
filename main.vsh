#version 420


layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;

out vec3 outPosition;
out vec3 outColor;
out vec3 outNormal;

// light matrices
uniform mat4 lightProjection, lightView;
out vec4 fragPositionFromLight;

// matrix transforms
uniform mat4 model, view, projection;

void main()
{
	outPosition = vec3(model * vec4(vertexPosition, 1.f));
	outColor = vertexColor;
	outNormal = mat3(transpose(inverse(model))) * vertexNormal;
	
	fragPositionFromLight = lightProjection * lightView * model * vec4(vertexPosition, 1.0);
	gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
}
