#version 450

#include "object.glsl"

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;

void main()
{
	outPosition = vec4(fragPosition, obj.metal);
	outNormal = vec4(fragNormal, obj.roughness);
}