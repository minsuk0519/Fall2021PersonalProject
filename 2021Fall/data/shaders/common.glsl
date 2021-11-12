layout(binding = 0) uniform Transform {
	mat4 worldToCamera;
	mat4 cameraToNDC;

	//will be moved
	mat4 objectMat;
} transform;