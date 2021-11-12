#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;

void main()
{
	outNormal = vec4(fragNormal, 1.0);
	outPosition = vec4(fragPosition, 1.0);
}