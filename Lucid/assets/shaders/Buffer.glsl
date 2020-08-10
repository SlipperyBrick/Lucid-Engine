#type vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Bitangent;
layout(location = 4) in vec2 a_TexCoord;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;

out VertexOutput
{
	vec2 TexCoord;

	vec3 Normal;
	vec3 FragPos;

	mat3 WorldNormals;

} vs_Output;

void main()
{
	vs_Output.Normal = mat3(u_Transform) * a_Normal;
	vs_Output.WorldNormals = mat3(u_Transform) * mat3(a_Tangent, a_Bitangent, a_Normal);

	// Flip texture coordinates
	vs_Output.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);

	vs_Output.FragPos = vec3(u_Transform * vec4(a_Position, 1.0));

	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 430 core

layout(location = 0) out vec4 o_Position;
layout(location = 1) out vec4 o_Normal;
layout(location = 2) out vec4 o_Albedo;
layout(location = 3) out vec4 o_Specular;

struct MaterialParameters
{
	vec3 Diffuse;
	vec3 Normal;

	float Specular;
	float Gloss;
};

MaterialParameters m_Params;

in VertexOutput
{
	vec2 TexCoord;

	vec3 Normal;
	vec3 FragPos;

	mat3 WorldNormals;

} vs_Input;

// Material texture inputs
uniform sampler2D u_DiffuseTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_SpecularTexture;
uniform sampler2D u_GlossTexture;

// Material inputs
uniform vec3 u_Diffuse;
uniform float u_Specular;
uniform float u_Gloss;

// ImGui texture toggles
uniform float u_DiffuseTexToggle;
uniform float u_NormalTexToggle;
uniform float u_SpecularTexToggle;
uniform float u_GlossTexToggle;

void main()
{	
	m_Params.Diffuse = u_DiffuseTexToggle > 0.5 ? texture(u_DiffuseTexture, vs_Input.TexCoord).rgb : u_Diffuse;
	m_Params.Specular = u_SpecularTexToggle > 0.5 ? texture(u_SpecularTexture, vs_Input.TexCoord).r : u_Specular;
	m_Params.Gloss = u_GlossTexToggle > 0.5 ? texture(u_GlossTexture, vs_Input.TexCoord).g : u_Gloss;

	if (u_NormalTexToggle > 0.5)
	{
		// Use texture maps normals
		m_Params.Normal = normalize(2.0 * texture(u_NormalTexture, vs_Input.TexCoord).rgb - 1.0);

		m_Params.Normal = normalize(vs_Input.WorldNormals * m_Params.Normal);
	}
	else
	{
		// Use mesh normals
		m_Params.Normal = normalize(vs_Input.Normal);
	}

	o_Position.rgb = vs_Input.FragPos;
	o_Position.a = 1.0;

	o_Normal.rgb = normalize(m_Params.Normal);
	o_Normal.a = 1.0;

	o_Albedo.rgb = m_Params.Diffuse;
	o_Albedo.a = 1.0;

	o_Specular.r = m_Params.Specular;
	o_Specular.g = m_Params.Gloss;
	o_Specular.b = 0.0;
	o_Specular.a = 1.0;
}