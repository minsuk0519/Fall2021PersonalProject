#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec2 fragPosition;

layout(location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2D samplerPosition;
layout (binding = 1) uniform sampler2D samplerNormal;

void main()
{
	outColor = texture(samplerPosition, fragTexCoord);
}