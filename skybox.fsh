#version 420

in vec3 texCoords;

out vec4 fragColor;

uniform samplerCube skybox;
uniform vec3 skyboxColor;

void main()
{
	fragColor = texture(skybox, texCoords) * vec4(skyboxColor, 0.0f);
}  