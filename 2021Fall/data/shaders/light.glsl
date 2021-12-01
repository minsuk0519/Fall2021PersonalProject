#include "common.glsl"

struct lightData {
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
};

layout(binding = 2) uniform lights {
	lightData lightsources[MAX_LIGHT];
	int lightNum;
};

vec3 computePointLight(vec3 surfacePos, vec3 normal, vec3 lightPos, lightData lightsource)
{
	vec3 surfaceToLight = lightPos - surfacePos;
	float lightDistance = length(surfaceToLight);
	vec3 lightDir = normalize(surfaceToLight);
	vec3 viewDir = normalize(surfacePos);
	vec3 reflectDir = normalize(lightDir - viewDir);

	vec3 ambient = lightsource.ambient;
	vec3 diffuse = lightsource.diffuse * max(dot(normal, lightDir), 0.0);
	vec3 specular = lightsource.specular * pow(max(dot(normal, reflectDir), 0.0), 32.0);

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

vec3 computeLight(vec3 surfacePos, vec3 normal)
{
	vec3 result;

	for(int i = 0; i < lightNum; ++i)
	{
		lightData lightsource = lightsources[i];

		if(lightsource.type == 0)
		{
			result += computePointLight(surfacePos, normal, lightsource.position, lightsource);
		}
		else if(lightsource.type == 1)
		{
			result += computeDirectionLight();
		}
		else if(lightsource.type == 2)
		{
			result += computeSpotLight();
		}
	}

	return result;
}

float ComputeNormalDistribution(float alpha, float ndoth)
{
	float alphasquare = alpha * alpha;
	float value = pow(ndoth * ndoth * (alphasquare - 1) + 1, 2.0);

	return alphasquare / (PI * value);
}

vec3 ComputeFresnelColor(vec3 color, float metal, float ndotv)
{
	vec3 baseF = mix(vec3(0.04, 0.04, 0.04), color, metal);
	float value = pow(clamp(1 - ndotv, 0.0, 1.0), 5.0);

	return baseF + (1 - baseF) * value;
}

float ComputeGeometryfunction(float roughness, float ndotv, float ndotl)
{
	//direct light
	float k = (roughness + 1) * (roughness + 1) / 8.0;

	float value1 = ndotv / (ndotv * (1.0-k) + k);
	float value2 = ndotl / (ndotl * (1.0-k) + k);

	return value1 * value2;
}

vec3 ComputeBRDF(vec3 viewDir, vec3 lightDir, vec3 normDir, float metal, float roughness, vec3 albedo)
{
	vec3 halfway = normalize(viewDir + lightDir);

	float roughnesssquare = roughness * roughness;
	float NdotL = clamp(dot(lightDir, normDir), 0.0, 1.0);
	float NdotH = clamp(dot(halfway, normDir), 0.0, 1.0);
	float NdotV = clamp(dot(viewDir, normDir), 0.0, 1.0);

	float D = ComputeNormalDistribution(roughnesssquare, NdotH);
	vec3 F = ComputeFresnelColor(albedo, metal, NdotV);
	float G = ComputeGeometryfunction(roughness, NdotV, NdotL);

	vec3 kD = vec3(1.0, 1.0, 1.0) - F;
	kD *= (1.0 - metal);

	vec3 specular = D * F * G / (4.0 * NdotV * NdotL + 0.00001);

	vec3 brdf = (kD * albedo / PI + specular) * NdotL;

	return brdf;
}

vec3 ComputePBR(vec3 view, vec3 norm, float metal, float roughness, vec3 albedo)
{
	vec3 result = vec3(0.0, 0.0, 0.0);

	for(int i = 0; i < lightNum; ++i)
	{
		lightData lightsource = lightsources[i];
		float dis = length(lightsource.position - view);
		vec3 viewDir = normalize(-view);
		vec3 lightDir = normalize(lightsource.position - view);
		vec3 normDir = normalize(norm);

		result += ComputeBRDF(viewDir, lightDir, normDir, metal, roughness, albedo);
	}

	//ambient
	result += albedo * 0.2;

	//light color
	//result *= vec3(1.0, 1.0, 1.0) / (dis * dis);

	//result = result / (result + vec3(1.0));
	//result = pow(result, vec3(1.0/2.2)); 

	return result;
}