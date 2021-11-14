#version 450

#include "settings.glsl"
#include "light.glsl"

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec2 fragPosition;

layout(location = 0) out vec4 outColor;

layout (binding = 3) uniform sampler2D texPosition;
layout (binding = 4) uniform sampler2D texNormal;

void main()
{
	if(setting.deferredType == 0)	
	{
		outColor = texture(texPosition, fragTexCoord);
		return;
	}
	else if(setting.deferredType == 1) 
	{
		outColor = texture(texNormal, fragTexCoord);
		return;
	}

	vec3 pos = texture(texPosition, fragTexCoord).rgb;
	vec3 norm = texture(texNormal, fragTexCoord).rgb;
	vec3 viewspacelightpos = (cam.worldToCamera * vec4(lightsource.position, 1.0)).xyz;

	if(setting.computationType == 0)
	{
		outColor = vec4(ComputePBR(pos, viewspacelightpos, norm), 1.0);
	}
	else
	{
		outColor = vec4(computeLight(pos, norm, viewspacelightpos), 1.0);
	}
}