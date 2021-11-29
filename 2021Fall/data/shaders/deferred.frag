#version 450

#include "settings.glsl"
#include "light.glsl"

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec2 fragPosition;

layout(location = 0) out vec4 outColor;

layout (binding = 3) uniform sampler2D texPosition;
layout (binding = 4) uniform sampler2D texNormal;
layout (binding = 5) uniform sampler2D texAlbedo;

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

	vec4 tex1 = texture(texPosition, fragTexCoord);
	vec4 tex2 = texture(texNormal, fragTexCoord);
	vec3 albedo = texture(texAlbedo, fragTexCoord).rgb;
	//albedo = pow(albedo, vec3(2.2));
	vec3 pos = tex1.rgb;
	float metal = tex1.a;
	vec3 norm = tex2.rgb;
	float roughness = tex2.a;
	vec3 viewspacelightpos = (cam.worldToCamera * vec4(lightsource.position, 1.0)).xyz;

	if(setting.computationType == 0)
	{
		outColor = vec4(ComputePBR(pos, viewspacelightpos, norm, metal, roughness, albedo), 1.0);
	}
	else
	{
		outColor = vec4(computeLight(pos, norm, viewspacelightpos), 1.0);
	}

	//outColor = vec4(texture(texAlbedo, fragTexCoord));
}