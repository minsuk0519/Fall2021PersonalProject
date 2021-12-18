#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 lightPosition;
layout(location = 2) in float lightPlane;

layout(location = 0) out float outputDepth;

void main()
{
	float lightDistance = length(fragPosition.xyz - lightPosition);

	lightDistance = lightDistance / lightPlane;

	gl_FragDepth = lightDistance;
	outputDepth = lightDistance;
}