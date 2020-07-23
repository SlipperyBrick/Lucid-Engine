#type vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;
uniform vec3 u_CameraPosition;

out VertexOutput
{
	vec2 TexCoord;
	vec3 Normal;
	vec3 FragPos;

} vs_Output;

void main()
{
	vs_Output.Normal = mat3(u_Transform) * a_Normal;

	// Flip texture coordinates
	vs_Output.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);

	vs_Output.FragPos = vec3(u_Transform * vec4(a_Position, 1.0));

	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 430 core

layout(location = 0) out vec4 colour;

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

struct MaterialParameters
{
	vec3 Diffuse;
	float Specular;
	vec3 Normal;
};

MaterialParameters m_Params;

in VertexOutput
{
	vec2 TexCoord;
	vec3 Normal;
	vec3 FragPos;

} vs_Input;

// Light inputs
uniform DirectionalLight r_DirectionalLight;

uniform PointLight r_PointLights[4];
uniform int r_PointLightCount;

// Material texture inputs
uniform sampler2D u_DiffuseTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_SpecularTexture;

// Material inputs
uniform vec3 u_DiffuseColour;
uniform float u_Specular;

// ImGui texture toggles
uniform float u_DiffuseTexToggle;
uniform float u_NormalTexToggle;
uniform float u_SpecularTexToggle;

// Camera position
uniform vec3 r_CameraPosition;

void main()
{	
	m_Params.Diffuse = u_DiffuseTexToggle > 0.5 ? texture(u_DiffuseTexture, vs_Input.TexCoord).rgb : u_DiffuseColour;
	m_Params.Specular = u_SpecularTexToggle > 0.5 ? texture(u_SpecularTexture, vs_Input.TexCoord).r : u_Specular;

	if (u_NormalTexToggle > 0.5)
	{
		// Use texture maps normals
		m_Params.Normal = normalize(2.0 * texture(u_NormalTexture, vs_Input.TexCoord).rgb - 1.0);
	}
	else
	{
		// Use mesh normals
		m_Params.Normal = normalize(vs_Input.Normal);
	}

	// Light attributes
	vec3 dirDiffuse = vec3(0.0);
	vec3 dirSpecular = vec3(0.0);
	vec3 dirAmbient = vec3(0.0);

	// Ambient constant
	dirAmbient += r_DirectionalLight.Ambient * m_Params.Diffuse;

	// Diffuse component
	vec3 lightDir = normalize(-r_DirectionalLight.Direction);
	float diff = max(dot(m_Params.Normal, lightDir), 0.0);
	dirDiffuse += r_DirectionalLight.Diffuse * diff * m_Params.Diffuse * r_DirectionalLight.Brightness;

	// Specular component
	float shininess = 32.0;
	vec3 viewDir = normalize(r_CameraPosition - vs_Input.FragPos);
	vec3 reflectDir = reflect(-lightDir, m_Params.Normal);

	// Specular factor
	vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(m_Params.Normal, halfwayDir), 0.0), shininess);
	dirSpecular += r_DirectionalLight.Specular * spec;

	// Light attributes
	vec3 pointDiffuse = vec3(0.0);
	vec3 pointSpecular = vec3(0.0);
	vec3 pointAmbient = vec3(0.0);

	// Iterate over point lights
	for (int i = 0; i < r_PointLightCount; i++)
	{
		// Ambient constant (cheap global illumination
		pointAmbient += r_PointLights[i].Ambient * m_Params.Diffuse;
	
		// Diffuse component
		vec3 lightDir = normalize(r_PointLights[i].Position - vs_Input.FragPos);
		float diff = max(dot(m_Params.Normal, lightDir), 0.0);
		pointDiffuse += r_PointLights[i].Diffuse * diff * m_Params.Diffuse * (r_PointLights[i].Brightness);
	
		// Specular component
		float shininess = 32.0;
		vec3 viewDir = normalize(r_CameraPosition - vs_Input.FragPos);
		vec3 reflectDir = reflect(-lightDir, m_Params.Normal);
	
		// Specular factor
		vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(m_Params.Normal, halfwayDir), 0.0), shininess);
		pointSpecular += r_PointLights[i].Specular * spec;
	
		// Attenuation
		float distance = length(r_PointLights[i].Position - vs_Input.FragPos);
		float attenuation = 1.0 / (r_PointLights[i].Brightness + (-r_PointLights[i].Falloff * distance + r_PointLights[i].Slope * (distance * distance)));    
		pointAmbient  *= attenuation; 
		pointDiffuse   *= attenuation;
		pointSpecular *= attenuation;
	
		// Accumulate lighting values
		dirAmbient += pointAmbient;
		dirDiffuse += pointDiffuse;
		dirSpecular += pointSpecular;
	}

	colour = vec4(dirDiffuse + dirSpecular + dirAmbient, 1.0f);
}