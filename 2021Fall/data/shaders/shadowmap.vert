#version 450

#include "common.glsl"
#include "object.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 offset;

void main()
{
	vec3 tempPos = (obj.objectMat * vec4(inPosition, 1.0)).xyz + offset;
	gl_Position = cam.cameraToNDC * vec4(tempPos, 1.0);

	fragNormal = normalize(mat3(transpose(inverse(cam.worldToCamera * obj.objectMat))) * inNormal);
}