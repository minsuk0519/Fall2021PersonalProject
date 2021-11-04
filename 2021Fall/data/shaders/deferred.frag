#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec2 fragPosition;

layout(location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2D texSampler;
layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerPosition;

void main()
{
	outColor = texture(samplerNormal, fragTexCoord);
}