#version 420

out vec4 fragColor;

uniform sampler2D hdrBuffer;
uniform float exposure;

void main()
{
	const float gamma = 2.2f;

	vec3 mapped = vec3(1.f) - exp(-hdrBuffer * exposure);
	mapped = pow(mapped, vec3(1.f / gamma));

  fragColor = vec4(mapped, 1.0);
}  