#version 450
layout (traingles) in;
layout (triangle_strip, max_vertice=18) out;

uniform mat4 lightMat[6];

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
		gl_Layer = face;
		for(int i = 0; i < 3; ++i)
		{
			fragPosition = gl_in[i].gl_Position;
			gl_Position = lightMat[face] * fragPosition;
			lightPosition = position;
			lightPlane = far_plane;
			EmitVertex();
		}
		EndPrimitive();
	}
}