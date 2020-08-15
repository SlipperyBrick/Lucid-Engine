#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;

	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 430

layout(location = 0) out vec4 vFragColor0;
layout(location = 1) out vec4 vFragColor1;
layout(location = 2) out vec4 vFragColor2;
 
in vec2 v_TexCoord;

// Depth blending output
uniform sampler2DRect depthBlenderTex;

// Front blending output
uniform sampler2DRect frontBlenderTex;

uniform float u_Alpha;	//fragment alpha

// Maximum depth value to clear the depth with
#define MAX_DEPTH 1.0

void main()
{
	vec4 vColor = vec4(0.4, 0.5, 0.5, 1.0);

	ivec2 texSize = textureSize(depthBlenderTex, 0);
	vec2 size = vec2(float(texSize.x), float(texSize.y));
	vec2 fragCoord = gl_FragCoord.xy / size;

	// Get the current fragments depth
	float fragDepth = gl_FragCoord.z;

	// Get the depth value from the depth blending texture
	vec2 depthBlender = texture(depthBlenderTex, gl_FragCoord.xy).xy;

	// Get the depth value from the front blending texture
	vec4 forwardTemp = texture(frontBlenderTex, gl_FragCoord.xy);

	// Depths and 1.0 - alpha always increase, we can use pass-through by default with max blending
	vFragColor0.xy = depthBlender;
	vFragColor0.a = 1.0;
	
	// Front colours always increase (destiniation + destination = source * alpha), we can use pass-through by default with max blending
	vFragColor1 = forwardTemp;
	
	// Because over blending makes the colour increase or decrease, pass-through cannot be used by default
	// For each layer peel only one fragment can have a colour greater than 0
	vFragColor2 = vec4(0.0);

	float nearestDepth = -depthBlender.x;
	float farthestDepth = depthBlender.y;
	float alphaMultiplier = 1.0 - forwardTemp.w;

	if (fragDepth < nearestDepth || fragDepth > farthestDepth)
	{
		// Skip this depth value in the dual depth-peeling algorithm
		vFragColor0.xy = vec2(-MAX_DEPTH);

		return;
	}
	
	if (fragDepth > nearestDepth && fragDepth < farthestDepth)
	{
		// This fragment needs to be peeled again
		vFragColor0.xy = vec2(-fragDepth, fragDepth);

		return;
	}	 

	// Fragment is now on the peeled layer from the prior pass
	// Shade it and make sure it isn't peeled again
	vFragColor0.xy = vec2(-MAX_DEPTH);
	
	// If the fragments depth is the nearest depth, we blend the colour to the second attachment
	if (fragDepth == nearestDepth)
	{
		vFragColor1.xyz += vColor.rgb * u_Alpha * alphaMultiplier;
		vFragColor1.w = 1.0 - alphaMultiplier * (1.0 - u_Alpha);
	}
	else
	{
		// If the fragments depth isn't the nearest depth, we write it to the third attachment
		vFragColor2 += vec4(vColor.rgb, u_Alpha);
	}
}