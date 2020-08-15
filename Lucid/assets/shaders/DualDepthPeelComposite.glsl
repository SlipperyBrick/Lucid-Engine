#type vertex
#version 430

layout(location = 0) in vec3 a_Position;

void main()
{
	gl_Position = vec4(a_Position.xy * 2 - 1.0 , 0, 1);
}

#type fragment
#version 430

uniform sampler2DRect depthBlenderTex;
uniform sampler2DRect frontBlenderTex;
uniform sampler2DRect backBlenderTex;

layout(location = 0) out vec4 vFragColor;

void main()
{
	// Get the front and back blender colours
	vec2 fragCoord = gl_FragCoord.xy;

	vec4 frontColour = texture(frontBlenderTex, fragCoord);
	vec3 backColour = texture(backBlenderTex, fragCoord).rgb; 

	// Composite the front and back blending results
	vFragColor.rgb = frontColour.rgb + backColour * frontColour.a;
	vFragColor.a = 1.0;
}