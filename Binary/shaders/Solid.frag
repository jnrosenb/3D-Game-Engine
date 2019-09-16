//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

#define spec_p 4
#define MAX_LIGHT_COUNT	8

vec3 diffuse_color = vec3(1, 1, 1);
vec3 specular_k = vec3(1, 1, 1);
int specExponent = 100;

layout(std140) uniform test_ubo
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
	vec4 lightColor[MAX_LIGHT_COUNT];		// 144  -272
	vec4 lightPos[MAX_LIGHT_COUNT];			// 272 - 400
	float lightRadius[MAX_LIGHT_COUNT];		// 400 - 528
};

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

//INPUT BLOCK
in VS_OUT
{
	vec2 texCoords;
	vec4 normalIn;
	vec4 view_dir;
	vec4 PositionLightSpace;
	vec4 lightToWorld[MAX_LIGHT_COUNT];
};

out vec4 frag_color;


//FUNCTION DECLARATIONS
float ShadowCalculation(vec4 posLightSpace);


//MAIN
void main(void) 
{		
	vec3 finalColor = vec3(0);
	vec3 diffuse = vec3(0);
	vec3 specular = vec3(0);
	
	diffuse_color = texture(diffuseTexture, texCoords).xyz;

	vec4 m = normalize(normalIn);
	
	vec3 ambient = 0.5 * diffuse_color;
	for (int i = 0; i < MAX_LIGHT_COUNT; ++i)
	{
		vec4 L = normalize(lightToWorld[i]);
		diffuse += max(dot(m, L), 0) * diffuse_color * lightColor[i].xyz;// * dwl;

		//vec4 r = 2 * m * max(dot(L, m), 0) - L;
		//r = normalize(r);
		//specular += pow(max(dot(r,L),0), spec_p) * specular_k * lightColor[i];// * dwl;
		vec4 v = normalize(view_dir);
		vec4 h = (L + v)/2;
		specular += pow(max(dot(h,m),0), spec_p) * specular_k * lightColor[i].xyz;// * dwl;
	}
	
    // calculate shadow
    float shadow = ShadowCalculation(PositionLightSpace);    

    finalColor = ambient + (1.0 - shadow) * (diffuse + specular); 
	//finalColor = ambient + diffuse + specular;
	frag_color = vec4(finalColor ,1);
}


float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
	// transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    
	// get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // check whether current frag pos is in shadow
	float bias = 0.0001;
	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0; 
	//float shadow = 0.0;
	//int sampleSize = 1;
	//vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	//for(int x = -sampleSize; x <= sampleSize; ++x)
	//{
	//	for(int y = -sampleSize; y <= sampleSize; ++y)
	//	{
	//		float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
	//		shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
	//	}    
	//}
	//shadow /= (2 * sampleSize + 1)*(2 * sampleSize + 1);

    return shadow;
}  

