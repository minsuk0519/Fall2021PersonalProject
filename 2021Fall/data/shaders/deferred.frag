#version 450

layout(binding = 0) uniform GUI {
	int deferredType;
} setting;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec2 fragPosition;

layout(location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D samplerPosition;
layout (binding = 2) uniform sampler2D samplerNormal;

void main()
{
	if(setting.deferredType == 0)	outColor = texture(samplerPosition, fragTexCoord);
	else if(setting.deferredType == 1) outColor = texture(samplerNormal, fragTexCoord);
}