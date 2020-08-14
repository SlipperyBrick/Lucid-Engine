#type vertex
#version 430

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main()
{
	vec4 position = vec4(a_Position.xy, 0.0, 1.0);

	v_TexCoord = a_TexCoord;

	gl_Position = position;
}

#type fragment
#version 430

layout(location = 0) out vec4 o_Colour;

in vec2 v_TexCoord;

uniform sampler2D u_LightingTexture;

uniform sampler2DRect u_depthTexture;
uniform sampler2DRect u_frontTexture;
uniform sampler2DRect u_backTexture;

uniform sampler2D u_EditorTexture;

uniform float u_Exposure;

void main()
{
	const float gamma = 2.2;
	const float pureWhite = 1.0;

	vec4 lightingPass;

	vec3 colour = texture(u_LightingTexture, v_TexCoord).rgb * u_Exposure;

	// Reinhard tonemapping operator
	float luminance = dot(colour, vec3(0.2126, 0.7152, 0.0722));
	float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);

	// Scale colour by ratio of average luminances
	vec3 mappedColour = (mappedLuminance / luminance) * colour;

	// Gamma correction to final output
	lightingPass = vec4(pow(mappedColour, vec3(1.0 / gamma)), 1.0);

	// Frag coords for sampling front and back textures
	vec2 fragCoord = gl_FragCoord.xy;

	// Get the front and back blender colours
	vec4 frontColour = texture(u_frontTexture, fragCoord);
	vec3 backColour = texture(u_backTexture, fragCoord).rgb;

	vec4 editorColour = texture(u_EditorTexture, v_TexCoord);

	o_Colour = vec4(frontColour.rgb + backColour.rgb * frontColour.a + lightingPass.rgb, 1.0);
}