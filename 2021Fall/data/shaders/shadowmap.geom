#version 450
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout (binding = 3) uniform lightProj {
	mat4 lightMat[6];

	vec3 position;
	float far_plane;
};

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 lightPosition;
layout(location = 2) out float lightPlane;

void main()
{
	for(int face = 0; face < 6; ++face)
	{
		for(int i = 0; i < 3; ++i)
		{
			gl_Layer = face;
			fragPosition = gl_in[i].gl_Position.xyz;
			gl_Position = lightMat[face] * vec4(fragPosition, 1.0);
			lightPosition = position;
			lightPlane = far_plane;
			EmitVertex();
		}
		EndPrimitive();
	}
}