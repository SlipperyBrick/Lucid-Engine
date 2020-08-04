#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main()
{
	vec4 position = vec4(a_Position.xy, 0.0, 1.0);

	v_TexCoord = a_TexCoord;

	gl_Position = position;
}

#type fragment
#version 430

layout(location = 0) out vec4 o_Colour;

struct DirectionalLight
{
    vec3 Direction;
	vec3 Diffuse;
    vec3 Ambient;
    vec3 Specular;

	float Brightness;
};

struct PointLight
{
	vec3 Position;
    vec3 Diffuse;
    vec3 Specular;

	float Brightness;
	float Quadratic;
};

in vec2 v_TexCoord;

// Light inputs
uniform DirectionalLight r_DirectionalLight;

uniform PointLight r_PointLights[4];
uniform int r_PointLightCount;

// Colour attachment inputs
uniform sampler2D u_PositionTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_SpecGlossTexture;

// Camera position
uniform vec3 u_CameraPosition;

void main()
{
    vec3 FragPos = texture(u_PositionTexture, v_TexCoord).rgb;
    vec3 Normal = texture(u_NormalTexture, v_TexCoord).rgb;
    vec3 DiffuseMap = texture(u_AlbedoTexture, v_TexCoord).rgb;
    float SpecularMap = texture(u_SpecGlossTexture, v_TexCoord).r;
	float GlossMap = texture(u_SpecGlossTexture, v_TexCoord).g;

	vec3 finalDiffuse = vec3(0.0);
	vec3 finalSpecular = vec3(0.0);

	vec3 dirAmbient = vec3(0.0);
	vec3 dirDiffuse = vec3(0.0);
	vec3 dirSpecular = vec3(0.0);

	// Ambient constant
	dirAmbient = r_DirectionalLight.Ambient * DiffuseMap;

	// Diffuse component
	vec3 lightDir = normalize(-r_DirectionalLight.Direction - FragPos);
	float NdotL = max(dot(Normal, lightDir), 0.0);
	dirDiffuse = r_DirectionalLight.Diffuse * NdotL * DiffuseMap * r_DirectionalLight.Brightness;

	// Specular component
	float shininess = 16.0;
	vec3 viewDir = normalize(u_CameraPosition - FragPos);

	// Specular factor
	vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);
	dirSpecular = r_DirectionalLight.Specular * spec * SpecularMap * GlossMap * r_DirectionalLight.Brightness;

	// Point light attributes
	vec3 pointDiffuse = vec3(0.0);
	vec3 pointSpecular = vec3(0.0);

	// Iterate over point lights
	for (int i = 0; i < r_PointLightCount; i++)
	{
		// Diffuse component
		vec3 lightDir = normalize(r_PointLights[i].Position - FragPos);
		float NdotL = max(dot(Normal, lightDir), 0.0);
		pointDiffuse += r_PointLights[i].Diffuse * NdotL * DiffuseMap * r_PointLights[i].Brightness;

		// Specular component
		float shininess = 16.0;
		vec3 viewDir = normalize(u_CameraPosition - FragPos);

		// Specular factor
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);
		pointSpecular += r_PointLights[i].Specular * spec * SpecularMap * GlossMap * r_PointLights[i].Brightness;
	
		// Attenuation
		float distance = length(r_PointLights[i].Position - FragPos);
		float attenuation = 1.0 / (r_PointLights[i].Brightness + r_PointLights[i].Quadratic * (distance * distance));
		pointDiffuse *= attenuation;
		pointSpecular *= attenuation;
	
		// Accumulate lighting values
		dirDiffuse += pointDiffuse;
		dirSpecular += pointSpecular;
	}

	vec3 lightingResult = dirAmbient + dirDiffuse + dirSpecular;

	o_Colour = vec4(lightingResult, 1.0);
}