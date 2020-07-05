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

uniform sampler2DMS u_Texture;

uniform int u_TextureSamples;

vec4 MultiSampleTexture(sampler2DMS tex, ivec2 texCoord, int samples)
{
    vec4 result = vec4(0.0);

    for (int i = 0; i < samples; i++)
	{
        result += texelFetch(tex, texCoord, i);
	}

    result /= float(samples);

    return result;
}

void main()
{
	const float gamma = 2.2;
	const float pureWhite = 1.0;

	ivec2 texSize = textureSize(u_Texture);
	ivec2 texCoord = ivec2(v_TexCoord * texSize);

	vec4 msColour = MultiSampleTexture(u_Texture, texCoord, u_TextureSamples);

	vec3 colour = msColour.rgb;

	// Reinhard tonemapping operator
	float luminance = dot(colour, vec3(0.2126, 0.7152, 0.0722));
	float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);

	// Scale colour by ratio of average luminances
	vec3 mappedColour = (mappedLuminance / luminance) * colour;

	// Gamma correction to final output
	o_Colour = vec4(pow(mappedColour, vec3(1.0 / gamma)), 1.0);
}