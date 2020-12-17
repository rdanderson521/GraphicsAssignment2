// Minimal fragment shader
// Iain Martin 2018

#version 420 core

#define MAX_LIGHTS 20

in VERTEX_OUT
{
	vec3 pos;
	vec3 normal;
	vec4 vertexColour;
	vec4 fragPosLightSpace[MAX_LIGHTS];
	vec2 texCoord;
} fIn;

out vec4 outputColor;

layout (std140) uniform lightParams	{
	vec3 lightPos[MAX_LIGHTS];
	uint lightMode[MAX_LIGHTS];
	mat4 lightSpace[MAX_LIGHTS];
	vec3 lightColour[MAX_LIGHTS];
	vec3 attenuationParams[MAX_LIGHTS];
	uint numLights;
} lights;

uniform vec3 viewPos;
uniform mat4 model, view, projection;
uniform mat3 normalMatrix;

uniform sampler2D shadowMap;

uniform float texThres[4];
uniform sampler2D tex[4];
uniform bool useTex;

uniform sampler2D roughness[4];
uniform bool useRoughness;

uniform float reflectiveness; // value of 0.01 - 1

vec3 specular_albedo = vec3(1., 0.9, 0.8);
vec3 global_ambient = vec3(0.1, 0.1, 0.1);

float shadowCalculation(vec4 lightSpace)
{

	vec3 projCoords = lightSpace.xyz / lightSpace.w;

	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowMap, projCoords.xy).r;

	float currentDepth = projCoords.z; 
	float bias = 0.005;
	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	if(projCoords.z > 1.0)
        shadow = 0.0;
    return shadow;
}


void main()
{
	vec4 colour;

	int texIdx = 0;
	if ( useTex)
	{
		int i = 3;
		while (i >= 0 && fIn.pos.y < texThres[i])
		{
			i--;
		}

		if (i >= 0)
		{
			texIdx = i;
			colour = texture(tex[texIdx], fIn.texCoord);
		}
		else 
		{
			texIdx = -1;
			colour = fIn.vertexColour;
		}
	}
	else
	{
		colour = fIn.vertexColour;
	}

	vec3 normal = fIn.normal;


	outputColor =  vec4((global_ambient * colour.xyz)  , 1.f);
	for (int i = 0; i < lights.numLights; i++)
	{
		vec4 position_h = vec4(fIn.pos, 1.0);
		vec3 light_pos3 = lights.lightPos[i].xyz;		
		
		vec3 currentLightColour = lights.lightColour[i];
		if (lights.lightColour[i] == vec3(0.f))
		{
			currentLightColour = vec3(1.0f);
		}

		vec3 ambient = colour.xyz  * 0.1;

		// Define our vectors to calculate diffuse and specular lighting
		mat4 mv_matrix = view * model;		// Calculate the model-view transformation
		vec4 P = view * position_h;	// Modify the vertex position (x, y, z, w) by the model-view transformation
		vec3 N = normalize(normalMatrix * normal);		// Modify the normals by the normal-matrix (i.e. to model-view (or eye) coordinates )
		vec3 L = -light_pos3;
		float distanceToLight = 0;
		if ( lights.lightMode[i] == 1)
		{
			L = light_pos3 - P.xyz;		// Calculate the vector from the light position to the vertex in eye space
			distanceToLight = length(L); // For attenuation
		}
		L = normalize(L);					// Normalise our light vector

		// Calculate the diffuse component
		vec3 diffuse = max(dot(N,L), 0.0) * colour.xyz;

		// Calculate the specular component using Phong specular reflection
		vec3 V = normalize(viewPos - P.xyz);	
		vec3 R = reflect(-L, N);
		vec3 specular = vec3(0.f);

		float shinyness = 0.f;
		float specularMultiplier = 0.2f;
		if (useRoughness && texIdx != -1)
		{
			float roughnessVal =  texture(roughness[texIdx], fIn.texCoord).r;

			 // use min to set max shinyness of 150 as sharp reflections look unrealistic on ground terrain
			shinyness = min(250.f/(2000.f * roughnessVal * roughnessVal + 0.01f), 150);

			specularMultiplier = specularMultiplier * (1.0 - roughnessVal);
		}
		else 
		{
			shinyness = reflectiveness;
		}

		if (shinyness != 0)
		{
			specular = specularMultiplier * pow(max(dot(R, V), 0.0),shinyness) * currentLightColour;
		}

		// Calculate the attenuation factor;
		float attenuation;
		if (lights.lightMode[i] == 0)
		{
			attenuation = 1.f;
		}
		else
		{
			attenuation = 1.0 / (lights.attenuationParams[i].x + lights.attenuationParams[i].y*distanceToLight + 
									   lights.attenuationParams[i].z * pow(distanceToLight, 2));
		}

		// calculate shadow value
		float shadow = 0.f;
		if (lights.lightMode[i] == 0)
		{
			shadow = shadowCalculation(fIn.fragPosLightSpace[i]);
		}

		outputColor +=  vec4(attenuation * (ambient + ((1.0 - shadow) * (specular + diffuse))), 1.0);
	}
}