#version 450

#include "common.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 2) in vec3 offset;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;

void main()
{
	vec3 tempPos = (transform.objectMat * vec4(inPosition, 1.0)).xyz + offset;
	fragPosition = (transform.worldToCamera * vec4(tempPos, 1.0)).xyz;
	gl_Position = transform.cameraToNDC * vec4(fragPosition, 1.0);

	fragNormal = normalize(mat3(transpose(inverse(transform.worldToCamera * transform.objectMat))) * inNormal);
}