#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform Transform {
	mat4 worldToCamera;
	mat4 cameraToNDC;

	//will be moved
	mat4 objectMat;
} transform;

void main()
{
	gl_Position = transform.cameraToNDC * transform.worldToCamera * transform.objectMat * vec4(inPosition, 1.0);
	fragColor = inColor;
}