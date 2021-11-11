#version 450

#include "common.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 3) in vec3 offset;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragPosition;

void main()
{
	vec3 tempPos = (transform.objectMat * vec4(inPosition, 1.0)).xyz;
	tempPos += offset;
	gl_Position = transform.cameraToNDC * transform.worldToCamera * vec4(tempPos, 1.0);
	fragColor = inColor;
	fragTexCoord = inTexCoord;
	fragPosition = (transform.objectMat * vec4(inPosition, 1.0)).xyz;
}