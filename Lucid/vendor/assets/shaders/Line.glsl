#type vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Colour;

uniform mat4 u_ViewProjection;

out vec4 v_Colour;

void main()
{
	v_Colour = a_Colour;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 430 core

layout(location = 0) out vec4 colour;

in vec4 v_Colour;

void main()
{
	colour = v_Colour;
}