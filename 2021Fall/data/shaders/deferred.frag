#version 450

#include "settings.glsl"

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec2 fragPosition;

layout(location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D texPosition;
layout (binding = 2) uniform sampler2D texNormal;

void main()
{
	if(setting.deferredType == 0)	outColor = texture(texPosition, fragTexCoord);
	else if(setting.deferredType == 1) outColor = texture(texNormal, fragTexCoord);
}