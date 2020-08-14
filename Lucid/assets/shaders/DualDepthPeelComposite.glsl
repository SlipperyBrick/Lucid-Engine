#type vertex
#version 430

layout(location = 0) in vec3 a_Position;

void main()
{
	gl_Position = vec4(a_Position.xy * 2 - 1.0 , 0, 1);
}

#type fragment
#version 430

uniform sampler2DRect depthBlenderTex;	//depth blending output
uniform sampler2DRect frontBlenderTex;	//front blending output
uniform sampler2DRect backBlenderTex;	//back blending output

layout(location = 0) out vec4 vFragColor;

void main()
{
	// Get the front and back blender colours
	vec2 fragCoord = gl_FragCoord.xy;

	vec4 frontColor = texture(frontBlenderTex, fragCoord);
	vec3 backColor = texture(backBlenderTex, fragCoord).rgb; 

	// Composite the front and back blending results
	vFragColor.rgb = frontColor.rgb + backColor * frontColor.a;
	vFragColor.a = 1.0;

	// Front blender
	//vFragColor.rgb = frontColor + vec3(alphaMultiplier);
	
	// Back blender
	//vFragColor.rgb = backColor;
}