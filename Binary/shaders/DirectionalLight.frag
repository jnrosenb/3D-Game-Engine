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
	int ScreenWidth;
	int ScreenHeight;
};

//Need the worldPos info and the shadowMap
uniform sampler2D GBufferPos;
uniform sampler2D GBufferNormals;
uniform sampler2D GBufferSpecGloss;
uniform sampler2D shadowMoments;

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
float MomentShadowCalculation(vec4 ClipSpaceLightpos);
vec3 CholeskyDecompSolver(float a11, float a12, float a13, 
	float a22, float a23, float a33, vec3 res);


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
    float shadow = ShadowIntensity * MomentShadowCalculation(PositionLightSpace);

	//FINAL COLOR
    finalColor = (1 - shadow) * (diffuse + specular); 
	frag_color = vec4(finalColor.xyz ,1);
}



//Moment shadow map
float MomentShadowCalculation(vec4 ClipSpaceLightpos)
{
    // perform perspective divide to clip space coords
    vec3 projCoords = ClipSpaceLightpos.xyz / ClipSpaceLightpos.w;

	//Get boundary conditions
	float threshold = 0.95;
	if (abs(projCoords.x) > threshold || abs(projCoords.y) > threshold)
		return 0.0;
		
	// transform to [0,1] range for uv
    projCoords = projCoords * 0.5 + 0.5;
	float zFrag = projCoords.z;

	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    vec4 moments = texture(shadowMoments, projCoords.xy);
	
	//Using the moments, calculate the vector b
	float alpha = 0.000005;
	vec4 b = (1.0 - alpha) * moments + alpha * vec4(0.5);
	vec3 c = CholeskyDecompSolver(1, b[0], b[1], b[1], b[2], b[3], vec3(1, zFrag, zFrag*zFrag));

	//Using c, solve quadratic formula
	// A=c[2]?, B=c[1], C=c[0]?
	float inv2a = 1.0 / (2 * c[2]);
	float sqrDisc = sqrt(c[1]*c[1] - 4*c[2]*c[0]);
	float s_1 = (-c[1] - sqrDisc) * inv2a;
	float s_2 = (-c[1] + sqrDisc) * inv2a;
	float top = max(s_1, s_2);
	float bottom = min(s_1, s_2);

	//Now evaluate
	if (zFrag <= bottom)
	{
		return 0.0;
	}
	else if (zFrag <= top)
	{
		float num = zFrag*top - b[0]*(zFrag + top) + b[1];
		float den = (top - bottom) * (zFrag - bottom);
		return num/den;
	}
	else
	{
		float num = bottom*top - b[0]*(bottom + top) + b[1];
		float den = (zFrag - top) * (zFrag - bottom);
		return 1.0 - num/den;
	}
}


vec3 CholeskyDecompSolver(float a11, float a12, float a13, 
	float a22, float a23, float a33, vec3 res) 
{
	//Get the terms for the triangulated matrix
	float a = 1;
	float b = a12;
	float c = a13;
	float invd = 1.0 / sqrt(a22 - b*b);
	float e = (a23 - b*c) * invd;
	float invf = 1.0 / sqrt(a33 - c*c - e*e);

	// Y <y1, y2, y3> is the intermediate
	// vector used in the decomposition
	float y1 = (res.x);
	float y2 = (res.y - b*y1) * invd;
	float y3 = (res.z - c*y1 - e*y2) * invf;

	//Calculate the terms we will return
	float c3 = y3 * invf; 
	float c2 = (y2 - e*c3) * invd;
	float c1 = (y1 - b*c2 - c*c3);
	return vec3(c1, c2, c3);
}
