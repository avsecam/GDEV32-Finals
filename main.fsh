#version 420


in vec3 outPosition;
in vec3 outColor;
in vec3 outNormal;
in vec4 fragPositionFromLight;

// final color
out vec4 fragColor;

uniform sampler2D shadowMap;
uniform samplerCube skybox;

const float AMBIENT_STRENGTH = 0.7f;

// POINT LIGHT STRUCT
struct PhongLighting
{
	vec3 ambient, diffuse, specular;
	vec3 position;
	vec3 direction;
	float coneInner, coneOuter;
};

// directional light
uniform vec3 directionalLightDirection;
uniform vec3 directionalLightAmbient;
uniform vec3 directionalLightDiffuse;
uniform vec3 directionalLightSpecular;
PhongLighting directionalLight =
{
	directionalLightAmbient,
	directionalLightDiffuse,
	directionalLightSpecular,
	vec3(0),
	directionalLightDirection,
	0, 0
};

// view position
uniform vec3 viewPosition;

const int POINT_LIGHT = 0;
const int DIRECTIONAL_LIGHT = 1;
const int SPOT_LIGHT = 2;
PhongLighting calculateLight(in PhongLighting light, in int lightType)
{
	// AMBIENT
	vec3 ambient = AMBIENT_STRENGTH * light.ambient;

	// normalized normals
	vec3 norm = normalize(outNormal);

	vec3 lightDirection;
	float attenuation;
	if(lightType != DIRECTIONAL_LIGHT)
	{
		lightDirection = normalize(light.position - outPosition);
		// calculate attenuation
		float distanceFragToLight = length(light.position - outPosition);
		attenuation = 1 / (1 + (0.14 * distanceFragToLight) + (0.07 * (distanceFragToLight * distanceFragToLight)));
	}
	else
	{
		lightDirection = normalize(-light.direction);
		attenuation = 1;
	}
	
	// DIFFUSE
	float diffuseStrength = max(dot(norm, lightDirection), 0.f);
	vec3 diffuse = diffuseStrength * light.diffuse * attenuation;

	// view and reflection
	vec3 viewDirection = normalize(viewPosition - outPosition);
	vec3 reflectDirection = reflect(-lightDirection, norm);

	// SPECULAR
	vec3 specular = pow(max(dot(reflectDirection, viewDirection), 0.0), 64.0f) * light.specular * attenuation;

	PhongLighting sum;
	if(lightType == SPOT_LIGHT)
	{	
		float theta = dot(-light.direction, lightDirection);
		// outside spotlight
		if(theta < cos(light.coneOuter))
		{
			sum = PhongLighting( ambient, vec3(0), vec3(0), vec3(0), vec3(0), 0, 0 );
		}
		// inside spotlight
		else
		{
			float epsilon = cos(light.coneInner) - cos(light.coneOuter);
			float spotLightIntensity = clamp((theta - cos(light.coneOuter)) / epsilon, 0, 1);
			sum = PhongLighting( ambient, diffuse * spotLightIntensity, specular * spotLightIntensity, vec3(0), vec3(0), 0, 0 );
		}
	}
	else
	{
		sum = PhongLighting( ambient, diffuse, specular, vec3(0), vec3(0), 0, 0 );
	}
	// sum = PhongLighting( ambient, vec3(0), vec3(0), vec3(0), vec3(0), 0, 0 );
	
	// SHADOWING
	vec3 fragLightNDC = fragPositionFromLight.xyz / fragPositionFromLight.w;
	fragLightNDC = (fragLightNDC + 1.f) / 2.f;

	float bias = max(0.0000125f * (1 - dot(outNormal, lightDirection)), 0.00001125f);
	float depthValue = texture(shadowMap, fragLightNDC.xy).x + bias;
	float currentDepth = fragLightNDC.z;

	if(depthValue < currentDepth)
	{
		float shadow = 0.f;
		vec2 texelSize = 1.f / textureSize(shadowMap, 0);
		for(int x = -1; x <= 1; ++x)
		{
			for(int y = -1; y <= 1; ++y)
			{
				float pcfDepth = texture(shadowMap, fragLightNDC.xy + vec2(x, y) * texelSize).r;
				shadow += (currentDepth - bias > pcfDepth) ? 1.f : 0.f;
			}
		}
		shadow /= 9.f;
		sum = PhongLighting( ambient + (1.f - shadow), vec3(0), vec3(0), vec3(0), vec3(0), 0, 0 );
	}
	else
	{
		sum = PhongLighting( ambient, diffuse, specular, vec3(0), vec3(0), 0, 0 );
	}
	return sum;
}

void main()
{
	// LIGHTING
	PhongLighting lights[1] = PhongLighting[]
	(
		calculateLight(directionalLight, DIRECTIONAL_LIGHT)
	);

	vec3 lightSum = vec3(1.f);

	vec3 ambientAverage = vec3(0);
	vec3 diffuseAndSpecularSum = vec3(0);
	for(int i = 0; i < lights.length(); i++)
	{
		ambientAverage += lights[i].ambient;
		diffuseAndSpecularSum += lights[i].diffuse + lights[i].specular;
	}
	ambientAverage = ambientAverage / lights.length();

	lightSum = ambientAverage + diffuseAndSpecularSum;

	// REFLECTION
	vec3 viewDirection = normalize(outPosition - viewPosition);
	vec3 reflection = reflect(viewDirection, normalize(outNormal));
	vec3 reflectionTexture = texture(skybox, reflection).rgb;

	vec3 finalColor = (lightSum) * outColor * reflectionTexture;
	fragColor = vec4(finalColor, 1.f);

	// debug
	// fragColor = vec4(vec3(fragLightNDC.z), 1.f);
	// fragColor = vec4(vec3(depthValue), 1.f);
}