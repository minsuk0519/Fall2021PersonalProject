layout(binding = 2) uniform lightData {
	vec3 ambient;
	float attenuationC1;
	vec3 diffuse;
	float attenuationC2;
	vec3 specular;
	float attenuationC3;


	vec3 position;
	float theta;
	vec3 direction;
	float phi;
	
	float falloff;

	int type;
} lightsource;

vec3 computePointLight(vec3 surfacePos, vec3 normal, vec3 lightPos)
{
	vec3 surfaceToLight = lightPos - surfacePos;
	float lightDistance = length(surfaceToLight);
	vec3 lightDir = normalize(surfaceToLight);
	vec3 viewDir = normalize(surfacePos);
	vec3 reflectDir = normalize(-lightDir + viewDir);

	vec3 ambient = lightsource.ambient;
	vec3 diffuse = lightsource.diffuse * max(dot(normal, -lightDir), 0.0);
	vec3 specular = lightsource.specular *
		pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

	float attenuation = 1.0f/(lightsource.attenuationC1 + lightsource.attenuationC2 * lightDistance + lightsource.attenuationC3 * lightDistance * lightDistance);

	return attenuation * ambient + attenuation * (diffuse + specular);
}

vec3 computeDirectionLight()
{
	return vec3(0,0,0);
}

vec3 computeSpotLight()
{
	return vec3(0,0,0);
}

vec3 computeLight(vec3 surfacePos, vec3 normal, vec3 lightPos)
{
	if(lightsource.type == 0)
	{
		return computePointLight(surfacePos, normal, lightPos);
	}
	else if(lightsource.type == 1)
	{
		return computeDirectionLight();
	}
	else if(lightsource.type == 2)
	{
		return computeSpotLight();
	}
	return vec3(0,0,0);
}