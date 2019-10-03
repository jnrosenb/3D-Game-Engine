//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

layout(std140) uniform test_gUBlock
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
};

//Need the worldPos info and the shadowMap
uniform sampler2D GBufferPos;
uniform sampler2D GBufferNormals;
uniform sampler2D GBufferSpecGloss;
uniform sampler2D shadowMap;

//Directional light information
uniform vec4 lightLook;
uniform vec3 lightColor;
uniform float Intensity;
uniform float ShadowIntensity;

//Input
in vec2 uvs;

//output
out vec4 frag_color;


//FUNCTION DECLARATIONS
float ShadowCalculation(vec4 posLightSpace);


//MAIN
void main(void) 
{
	vec3 finalColor = vec3(0);
	vec3 diffuse = vec3(0);
	vec3 specular = vec3(0);

	//FETCH STUFF FROM MAPS
	vec4 worldPos = texture(GBufferPos, uvs);
	vec4 normalIn = texture(GBufferNormals, uvs);
	vec3 specular_k = texture(GBufferSpecGloss, uvs).xyz;
	float specExponent = texture(GBufferSpecGloss, uvs).w;
	vec4 viewDir = eye - worldPos;
	vec4 PositionLightSpace = LightProjView * worldPos;

	vec4 m = normalize(normalIn);
	vec4 L = -lightLook;

	//DIFFUSE
	diffuse = max(dot(m, L), 0) * lightColor * Intensity;

	vec4 v = normalize(viewDir);
	vec4 h = normalize(L + v);
	//specular += pow(max(dot(h,m),0), specExponent) * specular_k * lightColor * Intensity;
	
    // calculate shadow
    float shadow = ShadowCalculation(PositionLightSpace);

	//FINAL COLOR
    finalColor = (1.0 - shadow) * (diffuse + specular); 
	frag_color = vec4(finalColor.xyz ,1);
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

    return ShadowIntensity * shadow;
}  

