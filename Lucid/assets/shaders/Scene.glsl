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
	vec3 TangentViewPos;
	vec3 TangentFragPos;

} vs_Output;

void main()
{
	vs_Output.Normal = a_Normal;

	// Flip texture coordinates
	vs_Output.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);

	vs_Output.FragPos = vec3(u_Transform * vec4(a_Position, 1.0));

    mat3 normalMatrix = transpose(inverse(mat3(u_Transform)));

	vec3 T = normalize(normalMatrix * a_Tangent);
	vec3 B = normalize(normalMatrix * a_Bitangent);
	vec3 N = normalize(normalMatrix * a_Normal);

	mat3 TBN = transpose(mat3(T, B, N));

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

void main()
{	
	m_Params.Diffuse = u_DiffuseTexToggle > 0.5 ? texture(u_DiffuseTexture, vs_Input.TexCoord).rgb : u_DiffuseColour; 

	colour = vec4(m_Params.Diffuse, 1.0f);
}