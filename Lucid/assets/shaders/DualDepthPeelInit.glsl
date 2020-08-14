#type vertex
#version 430

layout(location = 0) in vec3 a_Position;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;

void main()
{
	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 430

layout(location = 0) out vec4 o_FragmentDepth;
  
void main()
{
	// Set the fragment colour as negative fragment depth and positive fragment depth in the red and green channel
	// When combined with min/max blending this will help in peeling front and back layers simultaneously
	o_FragmentDepth.xy = vec2(-gl_FragCoord.z, gl_FragCoord.z);
}