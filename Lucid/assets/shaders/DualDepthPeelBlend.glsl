#type vertex
#version 430

layout(location = 0) in vec3 a_Position;

void main()
{
	gl_Position = vec4(a_Position.xy * 2 - 1.0 , 0, 1);
}

#type fragment
#version 430

// Intermediate blending result
uniform sampler2DRect tempTexture;

layout(location = 0) out vec4 vFragColor;

void main()
{
	// Return the intermediate blending result
	vFragColor = texture(tempTexture, gl_FragCoord.xy); 

	// If the alpha is 0 then discard the fragment
	if(vFragColor.a == 0)
	{
		discard;
	}
}