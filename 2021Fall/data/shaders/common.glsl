#define PI 3.141592
#define MAX_LIGHT 8

layout(binding = 0) uniform Camera {
	mat4 worldToCamera;
	mat4 cameraToNDC;
} cam;

