#type vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

struct Light
{
	vec3 Position;
	vec3 Radiance;
	float Multiplier;
};

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;
uniform vec3 u_CameraPosition;
uniform Light lights;

out VertexOutput
{
	vec2 TexCoord;
	vec3 Normal;
	vec3 FragPos;
	vec3 TangentLightPos;
	vec3 TangentViewPos;
	vec3 TangentFragPos;

} vs_Output;

void main()
{
	vs_Output.Normal = a_Normal;
	vs_Output.TexCoord = a_TexCoord;
	vs_Output.FragPos = vec3(u_Transform * vec4(a_Position, 1.0));

    mat3 normalMatrix = transpose(inverse(mat3(u_Transform)));

	vec3 T = normalize(normalMatrix * a_Tangent);
	vec3 B = normalize(normalMatrix * a_Binormal);
	vec3 N = normalize(normalMatrix * a_Normal);

	mat3 TBN = transpose(mat3(T, B, N));

	vs_Output.TangentLightPos = TBN * lights.Position;
	vs_Output.TangentViewPos = TBN * u_CameraPosition;
	vs_Output.TangentFragPos = TBN * vs_Output.FragPos;

	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 430 core

layout(location = 0) out vec4 colour;

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
	vec3 TangentLightPos;
	vec3 TangentViewPos;
	vec3 TangentFragPos;

} vs_Input;

// Material texture inputs
uniform sampler2D u_DiffuseTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_SpecularTexture;

uniform vec3 u_DiffuseColour;
uniform float u_Specular;

uniform float u_DiffuseTexToggle;
uniform float u_NormalTexToggle;
uniform float u_SpecularTexToggle;

vec4 Lighting()
{
	// Texture inputs
	m_Params.Diffuse = u_DiffuseTexToggle > 0.5 ? texture(u_DiffuseTexture, vs_Input.TexCoord).rgb : u_DiffuseColour; 
	m_Params.Specular = u_SpecularTexToggle > 0.5 ?  texture(u_SpecularTexture, vs_Input.TexCoord).r : u_Specular;

	// Normals
	m_Params.Normal = normalize(vs_Input.Normal);

	if (u_NormalTexToggle > 0.5)
	{
		m_Params.Normal = normalize(2.0 * texture(u_NormalTexture, vs_Input.TexCoord).rgb - 1.0);
	}

	m_Params.Normal = normalize(m_Params.Normal * 2.0 - 1.0);

	vec3 ambient = 0.1 * m_Params.Diffuse;

	vec3 lightDir = normalize(vs_Input.TangentLightPos - vs_Input.TangentFragPos);

	float diff = max(dot(lightDir, m_Params.Normal), 0.0);

	vec3 diffuse = diff * m_Params.Diffuse;

	vec3 viewDir = normalize(vs_Input.TangentViewPos - vs_Input.TangentFragPos);
	vec3 reflectDir = reflect(-lightDir, m_Params.Normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	float spec = pow(max(dot(m_Params.Normal, halfwayDir), 0.0), m_Params.Specular);

	vec3 specular = vec3(0.2) * spec;

	return vec4(ambient + diffuse + specular, 1.0);
}

void main()
{
	colour = Lighting();
}