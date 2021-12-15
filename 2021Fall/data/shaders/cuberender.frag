#version 450

#include "object.glsl"

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

void main()
{
	float roughness = 0.0;
	float metal = 0.0;
	outPosition = vec4(fragPosition, obj.metal - metal);
	outNormal = vec4(fragNormal, obj.roughness - roughness);
	outAlbedo = vec4(obj.color, 1.0);
}