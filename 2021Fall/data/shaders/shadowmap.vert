#version 450

#include "common.glsl"
#include "object.glsl"

layout(location = 0) in vec3 inPosition;

layout(location = 2) in vec3 offset;

void main()
{
	gl_Position = vec4((obj.objectMat * vec4(inPosition, 1.0)).xyz + offset, 1.0);
}