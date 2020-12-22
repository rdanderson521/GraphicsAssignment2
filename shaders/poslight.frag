//fragment shader with point and directional lighting with shadows and textures
//Ryan Anderson 2020

#version 420 core

#define MAX_LIGHTS 5
#define NUM_FAR_PLANES 3

in VERTEX_OUT
{
	vec3 pos;
	vec3 normal;
	vec4 vertexColour;
	vec4 fragLightSpace[MAX_LIGHTS*NUM_FAR_PLANES];
	vec2 texCoord;
	mat4 modelView;
	uint cascadingIdx;
} fIn;

out vec4 outputColor;

uniform vec3 viewPos;
uniform mat4 model, view, projection;
uniform mat3 normalMatrix;

//uniform sampler2D shadowMap;
uniform sampler2DArray shadowMapArr;
uniform float farPlanes[NUM_FAR_PLANES];


uniform sampler2D tex;
uniform bool useTex;

uniform sampler2D roughness;
uniform bool useRoughness;

uniform uint emitMode;
uniform vec3 emitColour;

layout (std140) uniform lightParams	{
	vec3 lightPos[MAX_LIGHTS];
	uint lightMode[MAX_LIGHTS];
	mat4 lightSpace[MAX_LIGHTS * NUM_FAR_PLANES];
	vec3 lightColour[MAX_LIGHTS];
	vec3 attenuationParams[MAX_LIGHTS];
	uint shadowIdx[MAX_LIGHTS];
	bool cascading[MAX_LIGHTS];
	uint numLights;
} lights;

uniform float reflectiveness; // value of 0.01 - 1


vec3 specular_albedo = vec3(1.0, 0.8, 0.6);
vec3 global_ambient = vec3(0.1, 0.1, 0.1);



float shadowCalculation(int lightIdx)
{
	vec4 lightSpace = fIn.fragLightSpace[lightIdx];

	vec3 projCoords = lightSpace.xyz / lightSpace.w;

	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = 0.f;

	if (lights.cascading[lightIdx] == true)
	{
		uint idx = 0;//fIn.cascadingIdx;
		
		lightSpace = fIn.fragLightSpace[idx];
		projCoords = lightSpace.xyz / lightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;

		closestDepth = texture(shadowMapArr, vec3(projCoords.xy, lights.shadowIdx[lightIdx] + idx)).r;
	}
	else
	{
		closestDepth = texture(shadowMapArr, vec3(projCoords.xy,lights.shadowIdx[lightIdx])).r;
	}

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
	if (useTex)
	{
		colour = texture(tex, fIn.texCoord);
	}
	else
	{
		colour = fIn.vertexColour;
	}

	vec3 normal = fIn.normal;

	vec3 emissive = vec3(0); // emmissive light component
	if (emitMode == 1)
	{
		if (emitColour != vec3(0.f))
		{
			emissive = emitColour;
		}
		else
		{
			emissive = vec3(1.0, 1.0, 0.8);
		}
	}


	outputColor =  vec4((global_ambient * colour.xyz) + emissive , 1.f);
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
		vec3 L = vec3(0.f);
		float distanceToLight = 0;
		if (lights.lightMode[i] == 0)
		{
			L = -vec3(fIn.modelView * vec4(light_pos3, 1.f)); // calculates the directional light vector in eye space
		}
		else if (lights.lightMode[i] == 1)
		{
			L = light_pos3 - P.xyz;		// Calculate the vector to the light position to the vertex in eye space
			distanceToLight = length(L); // For attenuation
		}
		L = normalize(L);					// Normalise our light vector

		// Calculate the diffuse component
		vec3 diffuse = max(dot(N, L), 0.0) * colour.xyz * (0.3 * vec3(1.f) + 0.7 * currentLightColour);

		// Calculate the specular component using Phong specular reflection
		vec3 V = normalize(viewPos - P.xyz);	
		vec3 R = reflect(-L, N);
		vec3 specular = vec3(0.f);

		float shinyness = 0.f;
		float specularMultiplier = 0.5f;
		if (useRoughness)
		{
			float roughnessVal = texture(roughness, fIn.texCoord).r;
			shinyness = 1500.f/(500.f * roughnessVal * roughnessVal + 0.01f);

			specularMultiplier = specularMultiplier * (1.0 - roughnessVal);
		}
		else 
		{
			shinyness = reflectiveness;
		}

		if (shinyness != 0)
		{
			specular = specularMultiplier * pow(max(dot(R, V), 0.0),shinyness) * specular_albedo;
		}


		// Calculate the attenuation factor;
		float attenuation;
//		if (lights.lightMode[i] == 0)
//		{
//			attenuation = 1.f;
//		}
//		else
		{
			// Define attenuation constants. These could be uniforms for greater flexibility
			attenuation = 1.0 / (lights.attenuationParams[i].x + lights.attenuationParams[i].y*distanceToLight + 
									   lights.attenuationParams[i].z * pow(distanceToLight, 2));
		}

		// calculate shadow value
		float shadow = 0.f;
		if (lights.lightMode[i] == 0)
		{
			shadow = shadowCalculation(i);
		}

		outputColor +=  vec4(attenuation * (ambient + ((1.0 - shadow) * (specular + diffuse)) ), 1.0);
	}
}