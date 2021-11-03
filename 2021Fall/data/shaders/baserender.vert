#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragPosition;

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
	fragTexCoord = inTexCoord;
	fragPosition = (transform.objectMat * vec4(inPosition, 1.0)).xyz;
}