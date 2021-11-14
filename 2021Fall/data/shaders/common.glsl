#define PI 3.141592

layout(binding = 0) uniform Camera {
	mat4 worldToCamera;
	mat4 cameraToNDC;
} cam;

