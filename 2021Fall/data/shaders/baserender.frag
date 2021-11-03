#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
	outColor = texture(texSampler, fragTexCoord);
	outNormal = vec4(fragTexCoord, 0.5, 1.0);
	outPosition = vec4(fragPosition, 1.0);
}