layout(binding = 0) uniform Camera {
	mat4 worldToCamera;
	mat4 cameraToNDC;
} cam;

layout(binding = 1) uniform ObjToWorld {
	mat4 objectMat;
} obj;
