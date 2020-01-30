//////////////////////////////////////////
// HEADER								//
//										//
// Name: Jose Rosenbluth Chiu			//
//                  					//
//////////////////////////////////////////

#version 330 core

//FUNCTION DECLARATIONS
float ShadowCalculation(vec4 worldPos, float viewDepth);

layout(std140) uniform test_gUBlock
{
	mat4 ProjView;							// 0   - 64
	mat4 LightProjView;						// 64   -128
	vec4 eye;								// 128  -144
	int ScreenWidth;
	int ScreenHeight;
};


//Need the worldPos info and the shadowMap
uniform sampler2D GBufferPos;
uniform sampler2D GBufferNormals;
uniform sampler2D GBufferSpecGloss;


//Cascaded shadow map sampler (for now, one sampler per shadow map)
uniform mat4 cameraView;
uniform sampler2D cascaded01;
uniform sampler2D cascaded02;
uniform sampler2D cascaded03;
uniform mat4 C_LPV[3];
uniform float C_depths[4];	// This is the min depth (in camera space) for choosing cascade
uniform int debugMode;

//Directional light information
uniform vec4 lightLook;
uniform vec3 lightColor;
uniform float Intensity;
uniform float ShadowIntensity;


//Input
in VS_OUT  
{
	vec2 uvs;
};

//output
out vec4 frag_color;


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

	//DIFFUSE
	vec4 m = normalize(normalIn);
	vec4 L = -lightLook;
	diffuse = max(dot(m, L), 0) * lightColor * Intensity;

	//SPECULAR
	vec4 v = normalize(viewDir);
	vec4 h = normalize(L + v);
	specular += pow(max(dot(h,m),0), specExponent) * specular_k * lightColor * Intensity;
	
    //SHADOW
	vec4 viewPos = cameraView*worldPos;
    float shadow = ShadowIntensity * ShadowCalculation(worldPos, viewPos.z);

	//FINAL COLOR
    finalColor = (1 - shadow) * (diffuse + 0*specular); //FOR NOW DO NOT USE SPECULAR
	frag_color = vec4(finalColor.xyz ,1);
	
	//DEBUG DRAW
	if(debugMode != 0)
	{
		if (abs(viewPos.z) < abs(C_depths[1]) && viewPos.z < C_depths[0])//index 0 is near
			frag_color += vec4(0.4, 0, 0 ,0);
		else if (abs(viewPos.z) < abs(C_depths[2]) && viewPos.z < C_depths[0])
			frag_color += vec4(0, 0.4, 0 ,0);
		else if (abs(viewPos.z) < abs(C_depths[3]) && viewPos.z < C_depths[0])//index 3 is Far
			frag_color += vec4(0, 0, 0.4 ,0);
	}
}



//Moment shadow map
float ShadowCalculation(vec4 worldPos, float viewDepth)
{	
	//Define variables to be used
	float CascadeDepth = 1.0;
	vec4 PositionLightSpace = vec4(0);
    vec3 projCoords = vec3(0);

	float banding = 0.0;

	//We should, based on the view space depth, select the correct cascade
	if (viewDepth < C_depths[0] && viewDepth > (C_depths[1] + banding))
	{
		//Bring to NDC and transform fragment depth to [0,1] range
		vec4 PositionLightSpace = C_LPV[0] * worldPos;
		projCoords = PositionLightSpace.xyz / PositionLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
		CascadeDepth = texture(cascaded01, projCoords.xy).r;
		
	}
	else if (viewDepth <= (C_depths[1] - banding) && viewDepth > (C_depths[2] + banding))
	{
		//Bring to NDC and transform fragment depth to [0,1] range
		vec4 PositionLightSpace = C_LPV[1] * worldPos;
		projCoords = PositionLightSpace.xyz / PositionLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
		CascadeDepth = texture(cascaded02, projCoords.xy).r;
	}
	else if (viewDepth < (C_depths[2] - banding) && viewDepth > C_depths[3])
	{
		//Bring to NDC and transform fragment depth to [0,1] range
		vec4 PositionLightSpace = C_LPV[2] * worldPos;
		projCoords = PositionLightSpace.xyz / PositionLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
		CascadeDepth = texture(cascaded03, projCoords.xy).r;
	}

	//Compare fragment to cascade depth. 0.0 is near, 1.0 is far
	float fragmentDepth = projCoords.z;
	float bias = 0.0;
	float shadow = fragmentDepth - bias > CascadeDepth  ? 1.0 : 0.0; 
	return shadow;
}
