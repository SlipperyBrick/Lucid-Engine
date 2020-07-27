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
	vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
	float Brightness;
	float Falloff;
	float Slope;
};

in vec2 v_TexCoord;

// Light inputs
uniform DirectionalLight r_DirectionalLight;

uniform PointLight r_PointLights[4];
uniform int r_PointLightCount;

// Colour attachment inputs
uniform sampler2D u_PositionTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_AlbedoSpecularTexture;

//uniform vec2 u_ViewportSize;

// Camera position
uniform vec3 r_CameraPosition;

void main()
{
    vec3 FragPos = texture(u_PositionTexture, v_TexCoord).rgb;
    vec3 Normal = texture(u_NormalTexture, v_TexCoord).rgb;
    vec3 Diffuse = texture(u_AlbedoSpecularTexture, v_TexCoord).rgb;
    float Specular = texture(u_AlbedoSpecularTexture, v_TexCoord).a;

	// Light attributes
	vec3 dirDiffuse = vec3(0.0);
	vec3 dirSpecular = vec3(0.0);
	vec3 dirAmbient = vec3(0.0);

	// Ambient constant
	dirAmbient += r_DirectionalLight.Ambient * Diffuse;

	// Diffuse component
	vec3 lightDir = normalize(-r_DirectionalLight.Direction - FragPos);
	float diff = max(dot(Normal, lightDir), 0.0);
	dirDiffuse += r_DirectionalLight.Diffuse * diff * Diffuse * r_DirectionalLight.Brightness;

	// Specular component
	float shininess = 16.0;
	vec3 viewDir = normalize(r_CameraPosition - FragPos);

	// Specular factor
	vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);
	dirSpecular += r_DirectionalLight.Specular * spec;

	// Light attributes
	vec3 pointDiffuse = vec3(0.0);
	vec3 pointSpecular = vec3(0.0);
	vec3 pointAmbient = vec3(0.0);

	// Iterate over point lights
	for (int i = 0; i < r_PointLightCount; i++)
	{
		// Ambient constant
		dirAmbient += r_PointLights[i].Ambient * Diffuse;

		// Diffuse component
		vec3 lightDir = normalize(r_PointLights[i].Position - FragPos);
		float diff = max(dot(Normal, lightDir), 0.0);
		dirDiffuse += r_PointLights[i].Diffuse * diff * Diffuse * r_DirectionalLight.Brightness;

		// Specular component
		float shininess = 16.0;
		vec3 viewDir = normalize(r_CameraPosition - FragPos);

		// Specular factor
		vec3 halfwayDir = normalize(lightDir + viewDir);  
		float spec = pow(max(dot(Normal, halfwayDir), 0.0), shininess);
		dirSpecular += r_PointLights[i].Specular * spec;
	
		// Attenuation
		float distance = length(r_PointLights[i].Position - FragPos);
		float attenuation = 1.0 / (r_PointLights[i].Brightness + (-r_PointLights[i].Falloff * distance + r_PointLights[i].Slope * (distance * distance)));    
		pointAmbient  *= attenuation; 
		pointDiffuse   *= attenuation;
		pointSpecular *= attenuation;
	
		// Accumulate lighting values
		dirAmbient += pointAmbient;
		dirDiffuse += pointDiffuse;
		dirSpecular += pointSpecular;
	}

	o_Colour = vec4(dirDiffuse + dirSpecular + dirAmbient, 1.0);

	vec3 position = texture(u_PositionTexture, v_TexCoord).rgb;
	o_Colour = vec4(position, 1.0);
	
	vec3 normal = texture(u_NormalTexture, v_TexCoord).rgb;
	o_Colour = vec4(normal, 1.0);

}